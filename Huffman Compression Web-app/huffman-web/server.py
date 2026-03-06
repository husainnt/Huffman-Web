import os
import subprocess
import json
from flask import Flask, render_template, request, send_file, jsonify
from werkzeug.utils import secure_filename

app = Flask(__name__)

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
EXE_PATH = os.path.join(BASE_DIR, "huffmancomp.exe")

UPLOAD_FOLDER = os.path.join(BASE_DIR, "uploads")
PROCESSED_FOLDER = os.path.join(BASE_DIR, "processed")

os.makedirs(UPLOAD_FOLDER, exist_ok=True)
os.makedirs(PROCESSED_FOLDER, exist_ok=True)


@app.route("/", methods=["GET", "POST"])
def index():

    if request.method == "POST":

        if not os.path.exists(EXE_PATH):
            return jsonify({"error": "huffmancomp.exe not found!"}), 500

        if "file" not in request.files:
            return jsonify({"error": "No file uploaded!"}), 400

        uploaded_file = request.files["file"]

        if uploaded_file.filename == "":
            return jsonify({"error": "No file selected!"}), 400

        filename = secure_filename(uploaded_file.filename)

        action = request.form.get("action", "compress")

        # Validate decompression file
        if action == "decompress" and not filename.endswith(".huff"):
            return jsonify({
                "error": "Please upload a .huff file for decompression"
            }), 400

        # Save file
        input_path = os.path.join(UPLOAD_FOLDER, filename)
        uploaded_file.save(input_path)

        # Determine output filename
        base_name = os.path.splitext(filename)[0]

        if action == "compress":
            output_filename = base_name + "_output.huff"

        else:
            # Decompression
            if base_name.endswith("_output"):
                base_name = base_name.replace("_output", "")

            output_filename = base_name + "_decompressed.txt"

        output_path = os.path.join(PROCESSED_FOLDER, output_filename)

        try:
            result = subprocess.run(
                [EXE_PATH, input_path, output_path, action],
                cwd=BASE_DIR,
                capture_output=True,
                text=True,
                check=True
            )

            stdout = result.stdout.strip()

            message = "Operation completed successfully"
            stats = None

            # Try parsing JSON stats from C++ program
            try:
                data = json.loads(stdout)

                if data.get("status") == "compressed":
                    message = "Compression Successful"
                    stats = data

                elif data.get("status") == "decompressed":
                    message = "Decompression Successful"

            except Exception:
                message = stdout if stdout else message

            return jsonify({
                "success": True,
                "message": message,
                "stats": stats,
                "download_url": "/download/" + output_filename
            })

        except subprocess.CalledProcessError as e:

            error_msg = e.stderr if e.stderr else "Huffman compression execution failed"

            return jsonify({
                "error": error_msg
            }), 500

    return render_template("index.html")


@app.route("/download/<filename>")
def download(filename):

    file_path = os.path.join(PROCESSED_FOLDER, filename)

    if os.path.exists(file_path):
        return send_file(file_path, as_attachment=True)

    return jsonify({"error": "File not found"}), 404


if __name__ == "__main__":
    app.run(debug=True)