# Wizard Duel Backend v2.0

FastAPI backend with Supabase integration for match recordings, user profiles, and player discovery.

## Setup

1.  **Install dependencies** using `uv`:
    ```bash
    uv pip install -r requirements.txt
    ```

2.  **Environment Variables**:
    Copy `.env.example` to `.env` and fill in your Supabase credentials.
    ```bash
    cp .env.example .env
    ```

3.  **Supabase Setup**:
    Run the SQL commands in `schema.sql` in your Supabase SQL Editor to create the necessary tables.
    Ensure you have a storage bucket named `match-files` created in Supabase Storage.

4.  **Run the server**:
    ```bash
    python server.py
    ```
    Or using `uvicorn`:
    ```bash
    uvicorn backend.server:app --reload --port 3000
    ```

## API Documentation
Once the server is running, visit:
- Swagger UI: `http://localhost:3000/docs`
- ReDoc: `http://localhost:3000/redoc`
