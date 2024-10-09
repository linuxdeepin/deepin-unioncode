# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

import tree_sitter_python as tspython
import tree_sitter_cpp as tscpp
import tree_sitter_java as tsjava
import tree_sitter_javascript as tsjavascript
import numpy as np
import uuid
import os
import sys
import hashlib
import sqlite3
from embedding import ONNXEmbeddingsProvider
from tree_sitter import Parser, Language
from typing import Generator, List, Dict, Any

supportedLanguages = {
    "cpp": "cpp",
    "hpp": "cpp",
    "cc": "cpp",
    "cxx": "cpp",
    "hxx": "cpp",
    "cp": "cpp",
    "hh": "cpp",
    "inc": "cpp",
    "c": "c",
    "h": "c",
    "js": "javascript",
    "py": "python",
    "pyw": "python",
    "pyi": "python",
    "java": "java",
}

def get_file_extension(filename):
    extension = os.path.splitext(filename)[1]
    return extension[1:] if extension.startswith('.') else extension

def get_parser_by_extension(extension):
    type = supportedLanguages.get(extension)
    if (type == "cpp" or type == "c"):
        parser = Parser(Language(tscpp.language()))
    elif (type == "python"):
        parser = Parser(Language(tspython.language()))
    elif (type == "java"):
        parser = Parser(Language(tsjava.language()))
    elif (type == "javascript"):
        parser = Parser(Language(tsjavascript.language()))
    else:
        return None
    return parser

class ChunkWithoutID:
    def __init__(self, content: str, start_line: int, end_line: int):
        self.content = content
        self.start_line = start_line
        self.end_line = end_line

def code_chunker(filepath: str, contents: str, max_chunk_size: int) -> Generator[ChunkWithoutID, None, None]:
    parser = get_parser_by_extension(get_file_extension(filepath))
    contentBytes = bytes(contents, "utf8")
    if (parser == None):
        return
    tree = parser.parse(contentBytes)

    FUNCTION_BLOCK_NODE_TYPES = ["block", "statement_block"]
    FUNCTION_DECLARATION_NODE_TYPES = [
        "method_definition",
        "function_definition",
        "function_item",
        "function_declaration",
        "method_declaration",
    ]

    def collapsed_replacement(node):
        if node.type == "statement_block":
            return "{ ... }"
        return "..."

    def first_child(node, grammar_name):
        if isinstance(grammar_name, list):
            return next((child for child in node.children if child.type in grammar_name), None)
        return next((child for child in node.children if child.type == grammar_name), None)

    def construct_class_definition_chunk(node):
        return collapse_children(
            node,
            ["block", "class_body", "declaration_list"],
            FUNCTION_DECLARATION_NODE_TYPES,
            FUNCTION_BLOCK_NODE_TYPES,
        )

    def construct_function_definition_chunk(node):
        body_node = node.children[-1]
        func_text = contentBytes[node.start_byte:body_node.start_byte].decode('utf8') + \
            collapsed_replacement(body_node)
        if (node.parent and node.parent.type in ["block", "declaration_list"] and
                node.parent.parent and node.parent.parent.type in ["class_definition", "impl_item"]):
            class_node = node.parent.parent
            class_block = node.parent
            return (f"{contentBytes[class_node.start_byte:class_block.start_byte].decode('utf8')}...\n\n"
                    f"{' ' * node.start_point[1]}{func_text}")
        return func_text

    collapsed_node_constructors = {
        "class_definition": construct_class_definition_chunk,
        "class_declaration": construct_class_definition_chunk,
        "impl_item": construct_class_definition_chunk,
        "function_definition": construct_function_definition_chunk,
        "function_declaration": construct_function_definition_chunk,
        "function_item": construct_function_definition_chunk,
        "method_declaration": construct_function_definition_chunk,
    }

    def collapse_children(node, block_types, collapse_types, collapse_block_types):
        code = contentBytes[:node.end_byte].decode('utf8')
        block = first_child(node, block_types)
        collapsed_children = []

        if block:
            children_to_collapse = [
                child for child in block.children if child.type in collapse_types]
            for child in reversed(children_to_collapse):
                grand_child = first_child(child, collapse_block_types)
                if grand_child:
                    start, end = grand_child.start_byte, grand_child.end_byte
                    collapsed_child = (code[child.start_byte:start] +
                                       collapsed_replacement(grand_child))
                    code = (code[:start] +
                            collapsed_replacement(grand_child) +
                            code[end:])
                    collapsed_children.insert(0, collapsed_child)

        code = code[node.start_byte:]
        return code

    def maybe_yield_chunk(node, root=True):
        if root or node.type in collapsed_node_constructors:
            chunk = ChunkWithoutID(
                content=node.text.decode('utf8'),
                start_line=node.start_point[0],
                end_line=node.end_point[0]
            )
            if len(chunk.content) < max_chunk_size:
                return chunk
        return None

    def get_smart_collapsed_chunks(node, root=True):
        chunk = maybe_yield_chunk(node, root)
        if chunk:
            yield chunk
            return

        if node.type in collapsed_node_constructors:
            collapsed_content = collapsed_node_constructors[node.type](node)
            yield ChunkWithoutID(
                content=collapsed_content,
                start_line=node.start_point[0],
                end_line=node.end_point[0]
            )

        for child in node.children:
            yield from get_smart_collapsed_chunks(child, False)

    yield from get_smart_collapsed_chunks(tree.root_node)


def chunk_document(filepath: str, contents: str, max_chunk_size: int) -> Generator[Dict[str, Any], None, None]:
    index = 0
    for chunk in code_chunker(filepath, contents, max_chunk_size):
        yield {
            "content": chunk.content,
            "startLine": chunk.start_line,
            "endLine": chunk.end_line,
            "index": index,
            "filepath": filepath,
            "digest": hashlib.md5(chunk.content.encode('utf-8')).hexdigest()
        }
        index += 1


def should_chunk(filepath: str, contents: str) -> bool:
    if len(contents) > 1000000 or len(contents) == 0:
        return False
    return '.' in filepath.split('/')[-1]


class DatabaseConnection:
    def __init__(self, db_path: str):
        self.db = sqlite3.connect(db_path)
        self.db.row_factory = sqlite3.Row

def getFileLastModidied(file_path):
    return os.path.getmtime(file_path)

def checkFileLastModified(file_path, db: sqlite3.Connection) -> bool:
    last_modified_time = getFileLastModidied(file_path)
    
    cursor = db.db.cursor()
    cursor.execute("SELECT last_modified FROM file_modification WHERE path = ?", (file_path,))
    db_last_modified_time = cursor.fetchone()

    if db_last_modified_time is None or db_last_modified_time[0] != last_modified_time:
        delete_lance_sql = f"""
        DELETE FROM lance_db_cache WHERE PATH = '{file_path}';
        """
        delete_chunks_sql = f"""
        DELETE FROM chunks WHERE PATH = '{file_path}';
        """
        cursor.execute(delete_lance_sql)
        cursor.execute(delete_chunks_sql)
        update_sql = """
        INSERT INTO file_modification (path, last_modified) VALUES (?, ?)
        ON CONFLICT(path) DO UPDATE SET last_modified = excluded.last_modified;
        """
        cursor.execute(update_sql, (file_path, last_modified_time))
        db.db.commit()
        return True
    else:
        return False

def _insert_chunks_sync(db: sqlite3.Connection, tag_string: str, chunks: List[Dict[str, Any]]):
    cursor = db.db.cursor()
    try:
        cursor.execute("BEGIN")

        chunks_sql = """
        INSERT INTO chunks (cacheKey, path, idx, startLine, endLine, content)
        VALUES (?, ?, ?, ?, ?, ?)
        """
#        chunk_tags_sql = """
#        INSERT INTO chunk_tags (chunkId, tag)
#        VALUES (last_insert_rowid(), ?)
#        """

        for chunk in chunks:
            cursor.execute(chunks_sql, (
                chunk["digest"],
                chunk["filepath"],
                chunk["index"],
                chunk["startLine"],
                chunk["endLine"],
                chunk["content"]
            ))
            if cursor.rowcount == 0:
                raise Exception("Failed to insert into chunks table")

#            cursor.execute(chunk_tags_sql, (tag_string,))
#            if cursor.rowcount == 0:
#                raise Exception("Failed to insert into chunk_tags table")

        db.db.commit()
    except Exception as e:
        db.db.rollback()
        raise e
    finally:
        cursor.close()

def _insert_embedding_sync(db: sqlite3.Connection, vector: bytes, chunk: Dict[str, Any]):
    cursor = db.db.cursor()
    try:
        cursor.execute("BEGIN")

        embedding_sql = """
            INSERT INTO lance_db_cache (uuid, cacheKey, path, artifact_id, vector, startLine, endLine, contents) 
            VALUES (?, ?, ?, ?, ?, ?, ?, ?)
        """

        cursor.execute(embedding_sql, (
            uuid.uuid4().hex,
            chunk["digest"],
            chunk["filepath"],
            #chunk["index"],
            "all-MiniLM-L6-v2", 
            vector,
            chunk["startLine"],
            chunk["endLine"],
            chunk["content"]
        ))

        if cursor.rowcount == 0:
            raise Exception("Failed to insert into embeddings table")

        db.db.commit()
    except Exception as e:
        db.db.rollback()
        raise e
    finally:
        cursor.close()

def initDb(db: sqlite3.Connection):
    db.db.execute("""
                CREATE TABLE IF NOT EXISTS chunks (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                cacheKey TEXT NOT NULL,
                path TEXT NOT NULL,
                idx INTEGER NOT NULL,
                startLine INTEGER NOT NULL,
                endLine INTEGER NOT NULL,
                content TEXT NOT NULL
                );
    """)
    db.db.execute("""
                CREATE TABLE IF NOT EXISTS file_modification (
                    path TEXT PRIMARY KEY,
                    last_modified REAL
                    );
    """)
    db.db.execute(""" 
                CREATE TABLE IF NOT EXISTS lance_db_cache (
                        uuid TEXT PRIMARY KEY,
                        cacheKey TEXT NOT NULL,
                        path TEXT NOT NULL,
                        artifact_id TEXT NOT NULL,
                        vector TEXT NOT NULL,
                        startLine INTEGER NOT NULL,
                        endLine INTEGER NOT NULL,
                        contents TEXT NOT NULL
                );
    """)
    db.db.commit()

def embeddingDirectory(dir: str, provider: ONNXEmbeddingsProvider, db: sqlite3.Connection):
    for entry in os.listdir(dir):
        full_path = os.path.join(dir, entry)
        if "/3rdparty/" in full_path or "/.unioncode/" in full_path:
            continue
        if os.path.isfile(full_path) and get_parser_by_extension(get_file_extension(full_path)) != None:
            needUpdate = checkFileLastModified(full_path, db)
            if (needUpdate == False):
                continue
            file = open(full_path, 'r')
            code = file.read()
            max_chunk_size = 1024
            chunks = list(chunk_document(full_path, code, max_chunk_size))
            _insert_chunks_sync(db, "test", chunks)

            for chunk in chunks:
                content = chunk['content']
                embedding = provider.embed_single(content)
                provider.code_blocks.append(content)
                _insert_embedding_sync(db, embedding.tobytes(), chunk)

        elif os.path.isdir(full_path):
            embeddingDirectory(full_path, provider, db)

if __name__ == "__main__":
    homePath = os.path.expanduser("~/")
    modelPath = sys.argv[1]
    db = DatabaseConnection(homePath + ".unioncode/index.sqlite")
    initDb(db)
    # embedding
    provider = ONNXEmbeddingsProvider(
        modelPath + '/all-MiniLM-L6-v2/onnx/model_quantized.onnx',
        modelPath + '/all-MiniLM-L6-v2/tokenizer.json'
    )

    if (len(sys.argv) > 2):
        embeddingDirectory(sys.argv[2], provider, db)

    db.db.close()