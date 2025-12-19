#!/usr/bin/env python3
"""
CGI Server - Executes C CGI programs as subprocess
Bridges HTML frontend to C backend executables
"""

from http.server import HTTPServer, BaseHTTPRequestHandler
from urllib.parse import urlparse, parse_qs
import json
import os
import subprocess
import traceback

class CGIHandler(BaseHTTPRequestHandler):
    
    def is_active(self, schedule_obj):
        """Return True if a schedule is considered active (not cancelled/rejected)."""
        try:
            status = (schedule_obj.get('status') or '').lower()
            return status not in ('rejected', 'cancelled')
        except Exception:
            return True

    def do_GET(self):
        parsed_path = urlparse(self.path)
        path = parsed_path.path
        query_params = parse_qs(parsed_path.query)

        self.send_response(200)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Content-type', 'application/json')
        self.end_headers()

        # Handle GET endpoints (pass-through to Python logic for now)
        if path == '/api/health':
            self.wfile.write(json.dumps({"status": "CGI Backend running"}).encode())
        elif path == '/api/pending_count':
            # Return count of pending requests
            schedules_file = os.path.join('/Users/shambhavisingh/codecrew', 'schedules.json')
            schedules = []
            if os.path.exists(schedules_file):
                try:
                    with open(schedules_file, 'r') as f:
                        schedules = json.load(f)
                except Exception:
                    schedules = []
            pending = len([s for s in schedules if (s.get('status') or '').lower() == 'pending'])
            self.wfile.write(json.dumps({"success": True, "pending": pending}).encode())
        elif path == '/api/classrooms':
            # Return classroom list with optional status populated from schedules.json
            classrooms = [
                {"id": 101, "name": "Room 101", "capacity": 60, "equipment": ["Projector", "Whiteboard"], "location": "Block B, 2nd Floor"},
                {"id": 102, "name": "Room 102", "capacity": 40, "equipment": ["Projector", "AC"], "location": "Block A, 1st Floor"},
                {"id": 103, "name": "Room 103", "capacity": 50, "equipment": ["Projector"], "location": "Block C, 3rd Floor"},
                {"id": 104, "name": "Room 104", "capacity": 30, "equipment": ["Whiteboard"], "location": "Block A, Ground"},
                {"id": 105, "name": "Room 105", "capacity": 80, "equipment": ["Projector", "AC"], "location": "Block B, 1st Floor"}
            ]
            # Load schedules and mark occupied/vacant (if any confirmed schedule exists)
            schedules_file = os.path.join('/Users/shambhavisingh/codecrew', 'schedules.json')
            schedules = []
            if os.path.exists(schedules_file):
                try:
                    with open(schedules_file, 'r') as f:
                        schedules = json.load(f)
                except Exception:
                    schedules = []

            for room in classrooms:
                room_schedules = [s for s in schedules if s.get('roomId') == room['id'] and self.is_active(s)]
                room['status'] = 'occupied' if len(room_schedules) > 0 else 'vacant'

            self.wfile.write(json.dumps({"success": True, "data": classrooms}).encode())
        elif path == '/api/dashboard':
            # Compute dashboard summary from schedules and classrooms
            classrooms_file = None
            classrooms = [101,102,103,104,105]
            schedules_file = os.path.join('/Users/shambhavisingh/codecrew', 'schedules.json')
            schedules = []
            if os.path.exists(schedules_file):
                try:
                    with open(schedules_file, 'r') as f:
                        schedules = json.load(f)
                except Exception:
                    schedules = []

            totalClassrooms = len(classrooms)
            occupiedRooms = set([s.get('roomId') for s in schedules if s.get('status') == 'confirmed'])
            pending = [s for s in schedules if s.get('status') == 'pending']
            faculties = set([s.get('faculty') for s in schedules if s.get('faculty')])

            summary = {
                'totalClassrooms': totalClassrooms,
                'occupiedClassrooms': len(occupiedRooms),
                'vacantClassrooms': totalClassrooms - len(occupiedRooms),
                'totalFaculty': len(faculties),
                'pendingBookings': len(pending)
            }
            self.wfile.write(json.dumps({"success": True, "data": summary}).encode())
        elif path == '/api/search/classroom':
            # Query param id
            room_id = None
            if 'id' in query_params:
                try:
                    room_id = int(query_params['id'][0])
                except Exception:
                    room_id = None
            classrooms = [
                {"id": 101, "name": "Room 101", "capacity": 60, "equipment": ["Projector", "Whiteboard"], "location": "Block B, 2nd Floor"},
                {"id": 102, "name": "Room 102", "capacity": 40, "equipment": ["Projector", "AC"], "location": "Block A, 1st Floor"},
                {"id": 103, "name": "Room 103", "capacity": 50, "equipment": ["Projector"], "location": "Block C, 3rd Floor"},
                {"id": 104, "name": "Room 104", "capacity": 30, "equipment": ["Whiteboard"], "location": "Block A, Ground"},
                {"id": 105, "name": "Room 105", "capacity": 80, "equipment": ["Projector", "AC"], "location": "Block B, 1st Floor"}
            ]
            room = next((r for r in classrooms if r['id'] == room_id), None)
            schedules_file = os.path.join('/Users/shambhavisingh/codecrew', 'schedules.json')
            schedules = []
            if os.path.exists(schedules_file):
                try:
                    with open(schedules_file, 'r') as f:
                        schedules = json.load(f)
                except Exception:
                    schedules = []

            room_schedules = [s for s in schedules if s.get('roomId') == room_id]
            # Exclude cancelled/rejected bookings from the returned classroom schedules
            room_schedules = [s for s in schedules if s.get('roomId') == room_id and self.is_active(s)]
            if room:
                room['schedules'] = room_schedules
                room['status'] = 'occupied' if len(room_schedules) > 0 else 'vacant'
                self.wfile.write(json.dumps({"success": True, "data": room}).encode())
            else:
                self.wfile.write(json.dumps({"success": False, "message": "Classroom not found"}).encode())
        elif path == '/api/search/vacancy':
            # time hour and day
            t = query_params.get('time', [None])[0]
            d = query_params.get('day', [None])[0]
            try:
                time_h = int(t) if t is not None else None
                day_n = int(d) if d is not None else None
            except Exception:
                time_h = None
                day_n = None

            classrooms = [
                {"id": 101, "name": "Room 101", "capacity": 60, "equipment": ["Projector", "Whiteboard"], "location": "Block B, 2nd Floor"},
                {"id": 102, "name": "Room 102", "capacity": 40, "equipment": ["Projector", "AC"], "location": "Block A, 1st Floor"},
                {"id": 103, "name": "Room 103", "capacity": 50, "equipment": ["Projector"], "location": "Block C, 3rd Floor"},
                {"id": 104, "name": "Room 104", "capacity": 30, "equipment": ["Whiteboard"], "location": "Block A, Ground"},
                {"id": 105, "name": "Room 105", "capacity": 80, "equipment": ["Projector", "AC"], "location": "Block B, 1st Floor"}
            ]
            schedules_file = os.path.join('/Users/shambhavisingh/codecrew', 'schedules.json')
            schedules = []
            if os.path.exists(schedules_file):
                try:
                    with open(schedules_file, 'r') as f:
                        schedules = json.load(f)
                except Exception:
                    schedules = []

            vacant = []
            # convert time_h to HHMM
            if time_h is not None:
                time_hhmm = time_h * 100
            else:
                time_hhmm = None

            for room in classrooms:
                occupied = False
                for s in schedules:
                    if not self.is_active(s):
                        continue
                    if s.get('roomId') == room['id'] and (day_n is None or s.get('day') == day_n):
                        st = s.get('startTime') or 0
                        et = s.get('endTime') or 0
                        if time_hhmm is None or (st <= time_hhmm and time_hhmm < et):
                            occupied = True
                            break
                if not occupied:
                    vacant.append(room)

            self.wfile.write(json.dumps({"success": True, "time": time_h, "day": day_n, "data": vacant}).encode())
        elif path == '/api/schedules':
            # Return persistent schedules from schedules.json if available
            schedules_file = os.path.join('/Users/shambhavisingh/codecrew', 'schedules.json')
            if os.path.exists(schedules_file):
                try:
                    with open(schedules_file, 'r') as f:
                        data = json.load(f)
                    self.wfile.write(json.dumps({"success": True, "data": data}).encode())
                except Exception as e:
                    self.wfile.write(json.dumps({"success": False, "message": f"Failed to read schedules: {str(e)}"}).encode())
            else:
                # Return empty schedules by default
                self.wfile.write(json.dumps({"success": True, "data": []}).encode())
        elif path == '/api/requests':
            # Pending requests list (return schedules with status 'pending' as request objects)
            try:
                schedules_file = os.path.join('/Users/shambhavisingh/codecrew', 'schedules.json')
                schedules = []
                if os.path.exists(schedules_file):
                    with open(schedules_file, 'r') as f:
                        schedules = json.load(f)
                requests = []
                for s in schedules:
                    if s.get('status') == 'pending':
                        req = {
                            'requestId': s.get('id'),
                            'bookingId': s.get('id'),
                            'requester': s.get('faculty') or 'unknown',
                            'action': 'booking_request',
                            'note': s.get('note', ''),
                            'roomId': s.get('roomId'),
                            'subject': s.get('subject'),
                            'startTime': s.get('startTime'),
                            'endTime': s.get('endTime'),
                            'day': s.get('day')
                        }
                        requests.append(req)
                self.wfile.write(json.dumps({"success": True, "data": requests}).encode())
            except Exception as e:
                self.wfile.write(json.dumps({"success": False, "message": str(e)}).encode())
        elif path.startswith('/api/faculty-location/'):
            # /api/faculty-location/{name}
            name = path.split('/')[-1]
            schedules_file = os.path.join('/Users/shambhavisingh/codecrew', 'schedules.json')
            schedules = []
            if os.path.exists(schedules_file):
                try:
                    with open(schedules_file, 'r') as f:
                        schedules = json.load(f)
                except Exception:
                    schedules = []
            # find first ACTIVE schedule for faculty (ignore cancelled/rejected)
            found = None
            for s in schedules:
                try:
                    if not self.is_active(s):
                        continue
                except Exception:
                    # if is_active fails for some malformed entry, skip it
                    continue
                if s.get('faculty') and s.get('faculty').lower() == name.lower():
                    found = {
                        'faculty': s.get('faculty'),
                        'classroom': s.get('roomId'),
                        'subject': s.get('subject'),
                        'time': s.get('startTime'),
                        'day': s.get('day')
                    }
                    break
            if found:
                self.wfile.write(json.dumps({"success": True, "data": found}).encode())
            else:
                self.wfile.write(json.dumps({"success": False, "message": "Faculty schedule not found"}).encode())
        elif path == '/api/faculty':
            schedules_file = os.path.join('/Users/shambhavisingh/codecrew', 'schedules.json')
            schedules = []
            if os.path.exists(schedules_file):
                try:
                    with open(schedules_file, 'r') as f:
                        schedules = json.load(f)
                except Exception:
                    schedules = []
            faculties = []
            seen = set()
            # Only list faculty who have active schedules (ignore cancelled/rejected)
            for s in schedules:
                try:
                    if not self.is_active(s):
                        continue
                except Exception:
                    # if is_active fails, skip this entry
                    continue
                name = s.get('faculty')
                if name and name not in seen:
                    seen.add(name)
                    faculties.append({'name': name, 'department': 'Unknown', 'email': f'{name.replace(" ", ".").lower()}@geu.ac.in'})
            self.wfile.write(json.dumps({"success": True, "data": faculties}).encode())
        else:
            self.send_error(404, "Not Found")

    def do_POST(self):
        content_length = int(self.headers.get('Content-Length', 0))
        body = self.rfile.read(content_length)
        
        parsed_path = urlparse(self.path)
        path = parsed_path.path

        self.send_response(200)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Content-type', 'application/json')
        self.end_headers()

        # Log incoming POST for debugging
        try:
            print(f"[CGI Server] POST {path} - {min(len(body), 200)} bytes")
            # attempt to show small preview of body
            try:
                preview = body.decode('utf-8', errors='ignore')
                print("[CGI Server] Body preview:\n", preview[:1000])
            except Exception:
                pass
        except Exception:
            pass

        try:
            # Route to C CGI executables
            if path == '/api/book':
                response = self.execute_cgi('./book_cgi', body)
                self.wfile.write(response)
            elif path == '/api/search':
                response = self.execute_cgi('./search_cgi', body)
                self.wfile.write(response)
            elif path == '/api/update':
                # If frontend is sending a simple update (id/status/role) update schedules.json directly
                try:
                    payload = json.loads(body.decode('utf-8')) if body else None
                except Exception:
                    payload = None

                if isinstance(payload, dict) and 'id' in payload and 'status' in payload:
                    try:
                        schedules_file = os.path.join('/Users/shambhavisingh/codecrew', 'schedules.json')
                        schedules = []
                        if os.path.exists(schedules_file):
                            with open(schedules_file, 'r') as f:
                                schedules = json.load(f)

                        booking_id = payload.get('id')
                        new_status = (payload.get('status') or '').lower()
                        found = None
                        for idx, s in enumerate(schedules):
                            if s.get('id') == booking_id:
                                found = (idx, s)
                                break

                        if not found:
                            self.wfile.write(json.dumps({"success": False, "message": "Booking not found"}).encode())
                        else:
                            idx, booking = found
                            # If cancelling or rejecting, remove from schedules and archive to cancellations.json
                            if new_status in ('cancelled', 'rejected'):
                                try:
                                    # remove from schedules
                                    removed = schedules.pop(idx)
                                    with open(schedules_file, 'w') as f:
                                        json.dump(schedules, f, indent=2)

                                    # append audit entry
                                    audit_file = os.path.join('/Users/shambhavisingh/codecrew', 'cancellations.json')
                                    audits = []
                                    if os.path.exists(audit_file):
                                        try:
                                            with open(audit_file, 'r') as af:
                                                audits = json.load(af)
                                        except Exception:
                                            audits = []
                                    actor = payload.get('requester') or payload.get('actor') or payload.get('role') or 'admin'
                                    audit_entry = {
                                        'bookingId': removed.get('id'),
                                        'roomId': removed.get('roomId'),
                                        'faculty': removed.get('faculty'),
                                        'subject': removed.get('subject'),
                                        'startTime': removed.get('startTime'),
                                        'endTime': removed.get('endTime'),
                                        'day': removed.get('day'),
                                        'action': new_status,
                                        'actor': actor,
                                        'timestamp': __import__('datetime').datetime.utcnow().isoformat() + 'Z'
                                    }
                                    audits.append(audit_entry)
                                    with open(audit_file, 'w') as af:
                                        json.dump(audits, af, indent=2)

                                    self.wfile.write(json.dumps({"success": True, "message": f"Booking {new_status} and archived", "audit": audit_entry}).encode())
                                except Exception as e:
                                    self.wfile.write(json.dumps({"success": False, "message": str(e)}).encode())
                            else:
                                # simple status update (e.g., confirm)
                                try:
                                    schedules[idx]['status'] = new_status
                                    with open(schedules_file, 'w') as f:
                                        json.dump(schedules, f, indent=2)
                                    self.wfile.write(json.dumps({"success": True, "message": "Status updated"}).encode())
                                except Exception as e:
                                    self.wfile.write(json.dumps({"success": False, "message": str(e)}).encode())
                    except Exception as e:
                        self.wfile.write(json.dumps({"success": False, "message": str(e)}).encode())
                else:
                    # Fallback to invoking the CGI executable
                    response = self.execute_cgi('./update_cgi', body)
                    self.wfile.write(response)
            # New: run the original interactive book program (compiled from book.c)
            elif path == '/api/run_book_cli':
                # Run the CLI binary (e.g. ./book). The POST body is passed to stdin.
                result_obj = self.execute_cli('./book', body)
                self.wfile.write(json.dumps(result_obj).encode())
            elif path == '/api/run_update_cli':
                # Run the CLI binary for update (e.g. ./update). The POST body is passed to stdin.
                result_obj = self.execute_cli('./update', body)
                self.wfile.write(json.dumps(result_obj).encode())
            elif path == '/api/persist_booking' or path == '/persist_booking':
                # Persist a booking to schedules.json (called after CLI reports success)
                try:
                    payload = json.loads(body.decode('utf-8'))
                    schedules_file = os.path.join('/Users/shambhavisingh/codecrew', 'schedules.json')
                    schedules = []
                    if os.path.exists(schedules_file):
                        with open(schedules_file, 'r') as f:
                            schedules = json.load(f)

                    # Normalize and validate numeric fields
                    try:
                        roomId = int(payload.get('roomId'))
                        startTime = int(payload.get('startTime'))
                        endTime = int(payload.get('endTime'))
                        day = int(payload.get('day'))
                    except Exception:
                        self.wfile.write(json.dumps({"success": False, "message": "Invalid numeric fields in payload"}).encode())
                        return

                    # Check for time clash in same classroom and day (ignore non-active bookings such as cancelled/rejected)
                    for s in schedules:
                        try:
                            # Skip entries that are not active (cancelled/rejected)
                            if not self.is_active(s):
                                continue
                            if int(s.get('roomId')) == roomId and int(s.get('day')) == day:
                                existing_start = int(s.get('startTime') or 0)
                                existing_end = int(s.get('endTime') or 0)
                                # overlap if not (new_end <= existing_start or new_start >= existing_end)
                                if not (endTime <= existing_start or startTime >= existing_end):
                                    self.wfile.write(json.dumps({"success": False, "message": f"Time clash with existing booking id {s.get('id')} (room {roomId})"}).encode())
                                    return
                        except Exception:
                            # ignore malformed entries
                            continue

                    # assign new id
                    next_id = 1
                    if isinstance(schedules, list) and len(schedules) > 0:
                        max_id = max((s.get('id', 0) for s in schedules))
                        next_id = max_id + 1
                    new_booking = {
                        'id': next_id,
                        'roomId': roomId,
                        'faculty': payload.get('faculty'),
                        'subject': payload.get('subject'),
                        'startTime': startTime,
                        'endTime': endTime,
                        'day': day,
                        'status': payload.get('status', 'confirmed')
                    }
                    schedules.append(new_booking)
                    with open(schedules_file, 'w') as f:
                        json.dump(schedules, f, indent=2)
                    self.wfile.write(json.dumps({"success": True, "data": new_booking}).encode())
                except Exception as e:
                    print(f"[CGI Server] persist_booking error: {e}")
                    self.wfile.write(json.dumps({"success": False, "message": str(e)}).encode())
            else:
                print(f"[CGI Server] POST endpoint not found: {path}")
                self.wfile.write(json.dumps({"success": False, "message": "Endpoint not found"}).encode())
        except Exception as e:
            print(f"[CGI Server] do_POST exception: {e}")
            self.wfile.write(json.dumps({"success": False, "message": str(e)}).encode())

    def do_PUT(self):
        # Support future PUT routes if needed
        self.send_response(200)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Content-type', 'application/json')
        self.end_headers()
        self.wfile.write(json.dumps({"success": False, "message": "PUT not implemented"}).encode())

    def do_DELETE(self):
        # Support future DELETE routes if needed
        self.send_response(200)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Content-type', 'application/json')
        self.end_headers()
        self.wfile.write(json.dumps({"success": False, "message": "DELETE not implemented"}).encode())

    def execute_cgi(self, cgi_path, request_body):
        """Execute C CGI program with request body as stdin"""
        try:
            # Ensure we invoke the executable via absolute path to avoid cwd issues
            base_dir = '/Users/shambhavisingh/codecrew'
            exec_path = cgi_path
            if not os.path.isabs(exec_path):
                exec_path = os.path.join(base_dir, exec_path.lstrip('./'))
            print(f"[CGI Server] Executing CGI: {exec_path}")
            result = subprocess.run(
                [exec_path],
                input=request_body,
                capture_output=True,
                timeout=5,
                cwd=base_dir
            )
            
            # C program outputs: "Content-Type: application/json\r\n\r\n{json}"
            output = result.stdout.decode('utf-8', errors='ignore')
            
            # Skip CGI headers and extract JSON
            if '\r\n\r\n' in output:
                json_part = output.split('\r\n\r\n', 1)[1]
            else:
                json_part = output
            
            return json_part.encode()
        except subprocess.TimeoutExpired:
            return json.dumps({"success": False, "message": "CGI timeout"}).encode()
        except Exception as e:
            return json.dumps({"success": False, "message": f"CGI error: {str(e)}"}).encode()

    def execute_cli(self, cli_path, request_body):
        """Execute a CLI binary (non-CGI) and return a JSON-able dict with its stdout/stderr.

        This is intended to run interactive console programs (like the original book compiled binary).
        The request_body (bytes) is passed to the program's stdin.
        """
        try:
            base_dir = '/Users/shambhavisingh/codecrew'
            exec_path = cli_path
            if not os.path.isabs(exec_path):
                exec_path = os.path.join(base_dir, exec_path.lstrip('./'))
            print(f"[CGI Server] Executing CLI: {exec_path}")
            result = subprocess.run(
                [exec_path],
                input=request_body,
                capture_output=True,
                timeout=10,
                cwd=base_dir
            )

            stdout = result.stdout.decode('utf-8', errors='ignore')
            stderr = result.stderr.decode('utf-8', errors='ignore')
            return {"success": True, "output": stdout, "error": stderr}
        except subprocess.TimeoutExpired:
            return {"success": False, "message": "CLI timeout"}
        except Exception as e:
            return {"success": False, "message": f"CLI error: {str(e)}"}

    def do_OPTIONS(self):
        self.send_response(200)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        self.end_headers()

    def log_message(self, format, *args):
        print(f"[CGI Server] {format % args}")

if __name__ == '__main__':
    PORT = 3000
    try:
        server = HTTPServer(('localhost', PORT), CGIHandler)
        print(f"🚀 CGI Backend Server running at http://localhost:{PORT}")
        print(f"📍 C executables (book_cgi, search_cgi, update_cgi) integrated")
        print(f"📡 API endpoints:")
        print(f"   - POST /api/book → book_cgi executable")
        print(f"   - POST /api/search → search_cgi executable")
        print(f"   - POST /api/update → update_cgi executable")
        print("Press Ctrl+C to stop\n")
        server.serve_forever()
    except Exception as e:
        print("[CGI Server] Failed to start server:")
        traceback.print_exc()
        print(f"Error: {e}")
        # exit non-zero to indicate failure
        raise
