import http.server
import json
import os
import subprocess

PORT = 8080
ROOT_DIR = os.path.dirname(os.path.abspath(__file__))
FRONTEND_DIR = os.path.join(ROOT_DIR, "frontend")
COMPILER_EXE = os.path.join(ROOT_DIR, "build", "Debug", "MyCompiler.exe")

class CompilerHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=FRONTEND_DIR, **kwargs)

    def do_POST(self):
        if self.path == '/api/compile':
            content_length = int(self.headers.get('Content-Length', 0))
            post_data = self.rfile.read(content_length).decode('utf-8')
            try:
                data = json.loads(post_data)
                source_code = data.get('source', '')

                if not os.path.exists(COMPILER_EXE):
                    response = {
                        "success": False,
                        "output": "",
                        "errors": f"Compiler binary not found at: {COMPILER_EXE}\nPlease build the project using CMake first.",
                        "phases": {}
                    }
                else:
                    out_exe = os.path.join(ROOT_DIR, "web_run.exe")
                    proc = subprocess.run(
                        [COMPILER_EXE, "--inspect", source_code, out_exe],
                        capture_output=True,
                        text=True,
                        cwd=ROOT_DIR
                    )

                    stdout = proc.stdout
                    stderr = proc.stderr
                    overall_success = (proc.returncode == 0)

                    phase_keys = ["lexer", "parser", "semantic", "llvm", "codegen"]
                    phase_tags = {
                        "lexer": "===PHASE:LEXER===",
                        "parser": "===PHASE:PARSER===",
                        "semantic": "===PHASE:SEMANTIC===",
                        "llvm": "===PHASE:LLVM===",
                        "codegen": "===PHASE:CODEGEN==="
                    }

                    phases = {}
                    failing_stage = None

                    for i, key in enumerate(phase_keys):
                        tag = phase_tags[key]
                        if tag in stdout:
                            start_idx = stdout.find(tag) + len(tag)
                            end_idx = len(stdout)
                            for next_key in phase_keys[i + 1:]:
                                next_tag = phase_tags[next_key]
                                if next_tag in stdout:
                                    end_idx = min(end_idx, stdout.find(next_tag))
                                    break
                            if "===OUTPUT===" in stdout:
                                out_idx = stdout.find("===OUTPUT===")
                                if out_idx > start_idx:
                                    end_idx = min(end_idx, out_idx)

                            phase_text = stdout[start_idx:end_idx].strip()
                            is_success = "Status: FAILED" not in phase_text
                            status_val = "success" if is_success else "failed"
                            if not is_success and failing_stage is None:
                                failing_stage = key

                            phases[key] = {
                                "status": status_val,
                                "content": phase_text
                            }
                        else:
                            phases[key] = {
                                "status": "not_run",
                                "content": "Phase was not executed because an earlier compiler stage encountered errors."
                            }

                    clean_output = ""
                    if "===OUTPUT===" in stdout:
                        clean_output = stdout.split("===OUTPUT===")[-1].strip()

                    response = {
                        "success": overall_success,
                        "failing_stage": failing_stage,
                        "output": clean_output,
                        "errors": stderr.strip() if not overall_success and stderr else "",
                        "phases": phases
                    }

                    for temp_file in [out_exe, out_exe + ".ll"]:
                        if os.path.exists(temp_file):
                            try:
                                os.remove(temp_file)
                            except OSError:
                                pass

                self.send_response(200)
                self.send_header('Content-Type', 'application/json')
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                self.wfile.write(json.dumps(response).encode('utf-8'))

            except Exception as e:
                self.send_response(500)
                self.send_header('Content-Type', 'application/json')
                self.end_headers()
                self.wfile.write(json.dumps({"success": False, "errors": str(e), "output": "", "phases": {}}).encode('utf-8'))
        else:
            self.send_error(404, "Endpoint not found")

    def do_OPTIONS(self):
        self.send_response(200)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'POST, GET, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        self.end_headers()

if __name__ == '__main__':
    server = http.server.ThreadingHTTPServer(('127.0.0.1', PORT), CompilerHandler)
    print(f"MyCompiler Web IDE Server running at http://localhost:{PORT}")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nServer stopped.")
