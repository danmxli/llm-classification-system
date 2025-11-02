from fastapi import FastAPI, HTTPException
from pydantic import BaseModel, Field
from typing import List, Dict
from functools import lru_cache
import numpy as np
import threading

# lazy-import heavy deps
_sent_model = None
_model_lock = threading.Lock()

def _load_model():
    global _sent_model
    if _sent_model is None:
        with _model_lock:
            if _sent_model is None:
                from sentence_transformers import SentenceTransformer
                # alternatives: "intfloat/e5-small-v2" (great for short labels), "all-MiniLM-L6-v2"
                _sent_model = SentenceTransformer("sentence-transformers/all-MiniLM-L6-v2")
    return _sent_model

def _normalize(v: np.ndarray) -> np.ndarray:
    n = np.linalg.norm(v, axis=-1, keepdims=True)
    n[n == 0.0] = 1.0
    return v / n

class ClassifyReq(BaseModel):
    text: str = Field(..., min_length=1)
    labels: List[str] = Field(..., min_items=1)

class ClassifyResp(BaseModel):
    best_label: str
    scores: Dict[str, float]  # cosine scores per label

app = FastAPI(title="Local Embedding Classifier", version="1.0.0")

@app.get("/healthz")
def healthz():
    return {"status": "ok"}

@lru_cache(maxsize=4096)
def _cached_label_embedding(label: str) -> np.ndarray:
    model = _load_model()
    vec = model.encode([label], normalize_embeddings=True)[0]
    return vec

@app.post("/classify", response_model=ClassifyResp)
def classify(req: ClassifyReq):
    labels = [l.strip() for l in req.labels if l.strip()]
    if not labels:
        raise HTTPException(status_code=400, detail="No non-empty labels provided.")
    # encode text once
    model = _load_model()
    text_vec = model.encode([req.text], normalize_embeddings=True)[0]

    # encode labels from cache
    label_vecs = np.vstack([_cached_label_embedding(lbl) for lbl in labels])

    # cosine similarity
    sims = label_vecs @ text_vec
    idx = int(np.argmax(sims))
    scores = {labels[i]: float(sims[i]) for i in range(len(labels))}
    return ClassifyResp(best_label=labels[idx], scores=scores)
