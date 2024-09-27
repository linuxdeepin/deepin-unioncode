# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

import onnxruntime as ort
import numpy as np
from typing import List, Dict, Tuple
from tokenizers import Tokenizer
from sklearn.metrics.pairwise import cosine_similarity
from datetime import datetime

class ONNXEmbeddingsProvider:
    def __init__(self, model_path='path/to/model_quantized.onnx', tokenizer_path='path/to/tokenizer.json', max_length=512):
        self.session = ort.InferenceSession(model_path)
        self.max_length = max_length

        # load tokenizer
        self.tokenizer = Tokenizer.from_file(tokenizer_path)
        self.tokenizer.enable_padding(length=self.max_length)
        self.tokenizer.enable_truncation(max_length=self.max_length)

        # get input and output
        self.input_names = [input.name for input in self.session.get_inputs()]
        self.output_name = self.session.get_outputs()[0].name

        # save and embedding
        self.code_blocks = []
        self.embeddings = []

    def tokenize(self, text: str) -> Dict[str, np.ndarray]:
        encoded = self.tokenizer.encode(text)
        return {
            'input_ids': np.array([encoded.ids], dtype=np.int64),
            'attention_mask': np.array([encoded.attention_mask], dtype=np.int64),
            'token_type_ids': np.array([encoded.type_ids], dtype=np.int64)
        }

    def embed_single(self, code_block: str) -> np.ndarray:
        inputs = self.tokenize(code_block)
        outputs = self.session.run([self.output_name], inputs)
        return outputs[0][0, 0, :]

    def add_code_blocks(self, code_blocks: List[str]):
        for block in code_blocks:
            embedding = self.embed_single(block)
            self.code_blocks.append(block)
            self.embeddings.append(embedding)

    def query(self, query_text: str, top_k: int = 5) -> List[Tuple[str, float]]:
        query_embedding = self.embed_single(query_text)
        # ensure embeddings is 2D array
        embeddings_array = np.array(self.embeddings)
        # Calculate the cosine similarity between the query embedding and all code block embeddings
        similarities = cosine_similarity([query_embedding], embeddings_array)[0]

        # get top_k result
        top_indices = np.argsort(similarities)[-top_k:][::-1]
        
        results = []
        for idx in top_indices:
            results.append((self.code_blocks[idx], similarities[idx]))
        
        return results
