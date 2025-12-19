# CGI Integration - TrackMyClass

## Overview
The TrackMyClass system now runs **all business logic in C** via CGI (Common Gateway Interface). The Python server acts as a lightweight CGI wrapper that executes pre-compiled C binaries.

## Architecture

```
┌─────────────────────────────────────┐
│   HTML Frontend (dashboard.html)    │
│   Browser: http://localhost:8000   │
└──────────────┬──────────────────────┘
               │ fetch() JSON requests
               ▼
┌──────────────────────────────────────┐
│   CGI Server (cgi_server.py)         │
│   http://localhost:3000              │
│   ├─ POST /api/book → book_cgi       │
│   ├─ POST /api/search → search_cgi   │
│   └─ POST /api/update → update_cgi   │
└──────────────┬───────────────────────┘
               │ subprocess.run()
               ▼
┌──────────────────────────────────────┐
│   C Executables                      │
│   ├─ ./book_cgi (compiled from C)    │
│   ├─ ./search_cgi (compiled from C)  │
│   └─ ./update_cgi (compiled from C)  │
│   stdin: JSON request                │
│   stdout: JSON response              │
└──────────────────────────────────────┘
```

## Compiled C Programs

### 1. **book_cgi** (34 KB)
- **Source**: `book_cgi.c`
- **Function**: Book classrooms
- **Input**: JSON with roomId, faculty, subject, startTime, endTime
- **Output**: Success/error JSON response
- **Usage**: `POST /api/book`

```bash
echo '{"roomId":101,"faculty":"Dr. Mehta","subject":"DSA","startTime":900,"endTime":1000}' | ./book_cgi
```

### 2. **search_cgi** (33 KB)
- **Source**: `search_cgi.c`
- **Function**: Search classrooms, faculty, vacancy
- **Input**: JSON with type (vacancy/classroom/faculty), parameters
- **Output**: Array of results in JSON
- **Usage**: `POST /api/search`

```bash
echo '{"type":"vacancy","time":9}' | ./search_cgi
echo '{"type":"classroom","id":101}' | ./search_cgi
echo '{"type":"faculty","name":"Dr. Mehta"}' | ./search_cgi
```

### 3. **update_cgi** (49 KB)
- **Source**: `update_cgi.c`
- **Function**: Update booking status (admin only)
- **Input**: JSON with id, status, role
- **Output**: Updated booking object
- **Usage**: `POST /api/update`

```bash
echo '{"id":5,"status":"confirmed","role":"admin"}' | ./update_cgi
```

## Quick Start

### Terminal 1: Start CGI Server
```bash
cd /Users/shambhavisingh/codecrew
python3 cgi_server.py
```

Output:
```
🚀 CGI Backend Server running at http://localhost:3000
📍 C executables (book_cgi, search_cgi, update_cgi) integrated
📡 API endpoints:
   - POST /api/book → book_cgi executable
   - POST /api/search → search_cgi executable
   - POST /api/update → update_cgi executable
```

### Terminal 2: Start Frontend Server
```bash
cd /Users/shambhavisingh/codecrew
python3 -m http.server 8000
```

### Browser
```
http://localhost:8000/auth.htm
```

## Testing

### Book a Classroom (C executable)
```bash
curl -s -X POST http://localhost:3000/api/book \
  -H 'Content-Type: application/json' \
  -d '{"roomId":101,"faculty":"Dr. Mehta","subject":"DSA","startTime":900,"endTime":1000}'
```

### Search Vacant Classrooms (C executable)
```bash
curl -s -X POST http://localhost:3000/api/search \
  -H 'Content-Type: application/json' \
  -d '{"type":"vacancy","time":9}'
```

### Update Booking Status (C executable)
```bash
curl -s -X POST http://localhost:3000/api/update \
  -H 'Content-Type: application/json' \
  -d '{"id":5,"status":"confirmed","role":"admin"}'
```

## File Structure

```
/Users/shambhavisingh/codecrew/
├── cgi_server.py            # CGI wrapper server
├── book_cgi                 # Compiled C executable
├── search_cgi               # Compiled C executable
├── update_cgi               # Compiled C executable
├── book_cgi.c               # C source
├── search_cgi.c             # C source
├── update_cgi.c             # C source
├── book.c                   # Original C program (reference)
├── search.c                 # Original C program (reference)
├── update.c                 # Original C program (reference)
├── dashboard.html           # HTML frontend
├── auth.htm                 # Auth page
└── README_CGI.md            # This file
```

## How CGI Works

1. **Browser sends HTTP POST request** with JSON body
2. **CGI Server (Python)** receives the request on /api/book, /api/search, /api/update
3. **CGI Server spawns C executable** using `subprocess.run()`
4. **Request body is piped** to C program via stdin
5. **C program parses JSON**, processes logic, outputs JSON response
6. **Response is sent** back to browser with Content-Type: application/json

## Key Features

✅ **Pure C Business Logic** - All algorithms in original C programs
✅ **No External Dependencies** - C uses only stdlib (stdio, string, etc.)
✅ **Fast Execution** - Compiled binaries execute in <100ms
✅ **JSON Serialization** - Manual JSON parse/generate (no external libs)
✅ **Security** - Role-based access control in C update_cgi
✅ **Stateless** - Each request is independent

## Debugging

### View server logs
```bash
tail -f /tmp/cgi_server.log
```

### Test C binary directly
```bash
echo '{"type":"vacancy","time":10}' | /Users/shambhavisingh/codecrew/search_cgi
```

### Check if CGI server is running
```bash
curl -s http://localhost:3000/api/health | python3 -m json.tool
```

## Limitations & Notes

- **State**: Each CGI call is stateless (no persistence across requests)
- **Performance**: Subprocess creation ~5-10ms overhead per request
- **Concurrency**: Python threading handles multiple requests
- **JSON Parsing**: Manual sscanf() - limited to simple structures
- **Error Handling**: Basic error messages; could add more detail

## Future Enhancements

- Add request logging in C
- Implement persistent storage (SQLite in C)
- Add email notifications
- Migrate to FastCGI for better performance
- Add more comprehensive error handling

---

**Integration Status**: ✅ **COMPLETE**
- C programs compiled ✓
- CGI server running ✓
- All endpoints tested ✓
- Frontend ready ✓
