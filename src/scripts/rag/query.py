# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

import uuid
import os
import sys
import sqlite3
import numpy as np
import json
from embedding import ONNXEmbeddingsProvider
from typing import Generator, List, Dict, Any
from sklearn.metrics.pairwise import cosine_similarity
from sklearn.metrics.pairwise import euclidean_distances

class DatabaseConnection:
    def __init__(self, db_path: str):
        self.db = sqlite3.connect(db_path)
        self.db.row_factory = sqlite3.Row

def getAllRows(db: sqlite3.Connection, workspace: str):
    cursor = db.cursor()
    table_name = "lance_db_cache"
    query_sql = f"SELECT * FROM {table_name} WHERE PATH LIKE '%{workspace}%';"
    cursor.execute(query_sql)
    rows = cursor.fetchall()
    return rows

class File:
    def __init__(self, file_path, file_content, file_name):
        self.file_path = file_path
        self.file_content = file_content
        self.file_name = file_name

    def to_dict(self):
        return {
            #"filePath": self.file_path,
            "fileName": self.file_name,
            "content": self.file_content
        }

if __name__ == "__main__":
    homePath = os.path.expanduser("~/")
    modelPath = sys.argv[1]
    db = DatabaseConnection(homePath + ".unioncode/index.sqlite")
    # embedding
    provider = ONNXEmbeddingsProvider(
        modelPath + '/all-MiniLM-L6-v2/onnx/model_quantized.onnx',
        modelPath + '/all-MiniLM-L6-v2/tokenizer.json'
    )

    itemMap = {}
    if (len(sys.argv) == 5):
        workspace = sys.argv[2]
        query = sys.argv[3]
        top_k = int(sys.argv[4])
        query_embedding = provider.embed_single(query)
        # get vector
        rows = getAllRows(db.db, workspace)
        embeddings = []
        for row in rows:
            itemMap[row['vector']] = row
            embedding = np.frombuffer(row['vector'], dtype=query_embedding.dtype)
            embeddings.append(embedding)
        if not embeddings:
            db.db.close()
            sys.exit()
        similarities = cosine_similarity([query_embedding], embeddings)[0]

        top_indices = np.argsort(similarities)[-top_k:][::-1]

        content = '''
        "Use the above code to answer the following question. You should not reference any files outside of what is shown, unless they are commonly known files, like a .gitignore or package.json. Reference the filenames whenever possible. If there isn't enough information to answer the question, suggest where the user might look to learn more.",
        '''
        results = []
        for idx in top_indices:
            file_path = rows[idx]['path']
            file_content = rows[idx]['contents']
            file_name = file_path + "[" + rows[idx]['startLine'].__str__() + "-" + rows[idx]['endLine'].__str__() + "]"
            file = File(file_path, file_content, file_name)
            obj = file.to_dict()
            obj["similarity"] = float(similarities[idx])
            results.append(obj)

        instruction = {
            "name": "Instructions",
            "description": "Instructions",
            "content": "Use the above code to answer the following question. You should not reference any files outside of what is shown, unless they are commonly known files, like a .gitignore or package.json. Reference the filenames whenever possible. If there isn't enough information to answer the question, suggest where the user might look to learn more."
        }

        jsonResult = {
            "Query": query,
            "Chunks": results,
            "Instructions": instruction
        }
        print(json.dumps(jsonResult, indent=4))
        
    db.db.close()