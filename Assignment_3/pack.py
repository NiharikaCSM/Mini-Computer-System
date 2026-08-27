import os

# Files and folders that waste Claude tokens
IGNORE_LIST = {
    'node_modules', '.git', '.DS_Store', 'venv', '.env', 
    '__pycache__', 'dist', 'build', '.idea', '.vscode',
    'package-lock.json', 'yarn.lock', 'pnpm-lock.yaml'
}

# Code files we want to capture
VALID_EXTENSIONS = {
    '.js', '.jsx', '.ts', '.tsx', '.py', '.java', '.c', '.cpp', 
    '.h', '.html', '.css', '.json', '.md', '.txt', '.sh', '.go', '.rs'
}

def pack_codebase(root_dir, output_file):
    with open(output_file, 'w', encoding='utf-8') as out:
        for root, dirs, files in os.walk(root_dir):
            # Exclude ignored directories safely
            dirs[:] = [d for d in dirs if d not in IGNORE_LIST]
            
            for file in files:
                if file in IGNORE_LIST:
                    continue
                
                ext = os.path.splitext(file)[1].lower()
                if ext in VALID_EXTENSIONS:
                    file_path = os.path.join(root, file)
                    relative_path = os.path.relpath(file_path, root_dir)
                    
                    out.write("\n--- START FILE: " + relative_path + " ---\n")
                    try:
                        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                            out.write(f.read())
                    except Exception as e:
                        out.write("[Could not read file: " + str(e) + "]\n")
                    out.write("\n--- END FILE: " + relative_path + " ---\n")

if __name__ == "__main__":
    current_directory = os.getcwd()
    output_filename = "claude-codebase.txt"
    print("Packing files inside: " + current_directory + "...")
    pack_codebase(current_directory, output_filename)
    print("Success! Created '" + output_filename + "' in this folder.")
