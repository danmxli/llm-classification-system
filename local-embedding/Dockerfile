FROM python:3.11-slim

# speed up model downloads at build-time
ENV SENTENCE_TRANSFORMERS_HOME=/root/.cache/sentence-transformers

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential git && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY requirements.txt ./
RUN pip install --no-cache-dir -r requirements.txt

COPY server.py ./

# warm up model
RUN python - <<'PY'
from sentence_transformers import SentenceTransformer
SentenceTransformer("sentence-transformers/all-MiniLM-L6-v2")
print("Model cached.")
PY

EXPOSE 8088
HEALTHCHECK --interval=30s --timeout=3s --retries=3 CMD \
    curl -fsS http://127.0.0.1:8088/healthz || exit 1

CMD ["uvicorn", "server:app", "--host", "0.0.0.0", "--port", "8088", "--workers", "1"]
