#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only or GPL-3.0-or-later
#
# Brief description of the software written in C or C++ or
#  or C# of Rust or Node.JS or a million other languages.
#
# Copyright (C) 2023 Andrew D. Harris <andrew2325@gmail.com>.
# Gemini was used to create this frontend to ect.  I am not a software engineer!
# This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <https://www.gnu.org/licenses/>.

# code...

import os
import subprocess
import tkinter as tk
from tkinter import filedialog, messagebox, ttk

class ECTGui(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Cetera")
        self.geometry("560x520")
        self.configure(bg="#1e1e1e")

        style = ttk.Style()
        style.theme_use("clam")

        title_label = tk.Label(self, text="Cetera - ECT & Archive Tool", fg="#ffffff", bg="#1e1e1e", font=("Helvetica", 13, "bold"))
        title_label.pack(pady=12)

        # Main dynamic container frame that swaps views
        self.container_frame = tk.Frame(self, bg="#1e1e1e")
        self.container_frame.pack(pady=5, fill="x", padx=20)

        # Build View 1: ECT Controls & Mode Switcher
        self.build_ect_view()

        # Build View 2: Archive Selection Menu
        self.build_archive_menu_view()

        # Start on ECT View
        self.show_ect_view()

        # Log Output view (shared at the bottom)
        log_frame = tk.Frame(self, bg="#1e1e1e")
        log_frame.pack(pady=10, fill="both", expand=True, padx=20)

        self.log_text = tk.Text(log_frame, bg="#2d2d2d", fg="#ffffff", insertbackground="white", relief="flat", font=("Courier", 9))
        self.log_text.pack(side="left", fill="both", expand=True)

        scrollbar = tk.Scrollbar(log_frame, command=self.log_text.yview)
        scrollbar.pack(side="right", fill="y")
        self.log_text.config(yscrollcommand=scrollbar.set)

    def build_ect_view(self):
        self.ect_view_frame = tk.Frame(self.container_frame, bg="#1e1e1e")

        # Options Frame (ECT Level & Strip)
        opt_frame = tk.Frame(self.ect_view_frame, bg="#1e1e1e")
        opt_frame.pack(pady=5, fill="x")

        tk.Label(opt_frame, text="ECT Level (1-9):", fg="#cccccc", bg="#1e1e1e").pack(side="left", padx=5)
        self.level_spin = tk.Spinbox(opt_frame, from_=1, to=9, width=4)
        self.level_spin.delete(0, "end")
        self.level_spin.insert(0, "9")
        self.level_spin.pack(side="left", padx=5)

        self.strip_var = tk.BooleanVar(value=True)
        strip_chk = tk.Checkbutton(opt_frame, text="Strip Metadata", variable=self.strip_var, fg="#ffffff", bg="#1e1e1e", selectcolor="#333333", activebackground="#1e1e1e", activeforeground="#ffffff")
        strip_chk.pack(side="right", padx=5)

        # Action Buttons Frame
        btn_frame = tk.Frame(self.ect_view_frame, bg="#1e1e1e")
        btn_frame.pack(pady=10)

        file_btn = tk.Button(btn_frame, text="Select Files (ECT)", command=self.process_files, bg="#007acc", fg="white", padx=8, pady=6, relief="flat", font=("Helvetica", 9, "bold"))
        file_btn.pack(side="left", padx=4)

        dir_btn = tk.Button(btn_frame, text="Select Directory (ECT)", command=self.process_directory, bg="#28a745", fg="white", padx=8, pady=6, relief="flat", font=("Helvetica", 9, "bold"))
        dir_btn.pack(side="left", padx=4)

        switch_btn = tk.Button(btn_frame, text="Archive Menu...", command=self.show_archive_menu_view, bg="#d97706", fg="white", padx=8, pady=6, relief="flat", font=("Helvetica", 9, "bold"))
        switch_btn.pack(side="left", padx=4)

    def build_archive_menu_view(self):
        self.archive_menu_frame = tk.Frame(self.container_frame, bg="#1e1e1e")

        tk.Label(self.archive_menu_frame, text="Archive Creator", fg="#ffffff", bg="#1e1e1e", font=("Helvetica", 11, "bold")).pack(pady=5)

        # Format Selection Row
        fmt_frame = tk.Frame(self.archive_menu_frame, bg="#1e1e1e")
        fmt_frame.pack(pady=5)

        tk.Label(fmt_frame, text="Format:", fg="#cccccc", bg="#1e1e1e").pack(side="left", padx=5)
        self.archive_format_var = tk.StringVar(value="7z")
        tk.Radiobutton(fmt_frame, text="p7zip (.7z)", variable=self.archive_format_var, value="7z", fg="#ffffff", bg="#1e1e1e", selectcolor="#333333", activebackground="#1e1e1e", activeforeground="#ffffff").pack(side="left", padx=5)
        tk.Radiobutton(fmt_frame, text="tar.xz (.tar.xz)", variable=self.archive_format_var, value="tar.xz", fg="#ffffff", bg="#1e1e1e", selectcolor="#333333", activebackground="#1e1e1e", activeforeground="#ffffff").pack(side="left", padx=5)

        # Archive Action Buttons
        arc_btn_frame = tk.Frame(self.archive_menu_frame, bg="#1e1e1e")
        arc_btn_frame.pack(pady=10)

        arc_files_btn = tk.Button(arc_btn_frame, text="Select Files to Archive", command=self.archive_selected_files, bg="#007acc", fg="white", padx=10, pady=6, relief="flat", font=("Helvetica", 9, "bold"))
        arc_files_btn.pack(side="left", padx=6)

        arc_dir_btn = tk.Button(arc_btn_frame, text="Select Directory to Archive", command=self.archive_selected_directory, bg="#28a745", fg="white", padx=10, pady=6, relief="flat", font=("Helvetica", 9, "bold"))
        arc_dir_btn.pack(side="left", padx=6)

        back_btn = tk.Button(arc_btn_frame, text="< Back", command=self.show_ect_view, bg="#555555", fg="white", padx=10, pady=6, relief="flat", font=("Helvetica", 9, "bold"))
        back_btn.pack(side="left", padx=6)

    def show_ect_view(self):
        self.archive_menu_frame.pack_forget()
        self.ect_view_frame.pack(fill="x", expand=True)

    def show_archive_menu_view(self):
        self.ect_view_frame.pack_forget()
        self.archive_menu_frame.pack(fill="x", expand=True)

    def log(self, message):
        self.log_text.insert("end", message + "\n")
        self.log_text.see("end")

    def process_files(self):
        files = filedialog.askopenfilenames(
            title="Select Files to Optimize",
            filetypes=[("Supported Files", "*.png *.jpg *.jpeg *.zip *.gz"), ("All Files", "*.*")]
        )
        if not files:
            return
        self.run_ect_compression(list(files), "selected files")

    def process_directory(self):
        directory = filedialog.askdirectory(title="Select Directory to Optimize")
        if not directory:
            return

        supported_exts = (".png", ".jpg", ".jpeg", ".zip", ".gz")
        files_to_process = []
        
        for root, _, filenames in os.walk(directory):
            for filename in filenames:
                if filename.lower().endswith(supported_exts):
                    files_to_process.append(os.path.join(root, filename))

        if not files_to_process:
            messagebox.showinfo("No Files", "No supported files found in the selected directory.")
            return

        self.run_ect_compression(files_to_process, directory)

    def run_ect_compression(self, files_to_process, source_desc):
        if subprocess.run(["which", "ect"], capture_output=True).returncode != 0:
            messagebox.showerror("Error", "'ect' command not found.\nInstall it via source or your package manager.")
            return

        level = f"-{self.level_spin.get()}"
        strip = "-strip" if self.strip_var.get() else None

        self.log(f"Starting ECT compression for {len(files_to_process)} items from {source_desc} at level {level}...")
        self.update()

        success_count = 0
        fail_count = 0

        for file in files_to_process:
            cmd = ["ect", level]
            if strip:
                cmd.append(strip)
            cmd.append(file)

            while True:
                self.log(f"Executing: {' '.join(cmd)}")
                self.update()

                res = subprocess.run(cmd, capture_output=True, text=True)
                if res.returncode == 0:
                    self.log(f"Success: {os.path.basename(file)}")
                    success_count += 1
                    break
                else:
                    err = res.stderr.strip() or res.stdout.strip()
                    self.log(f"Error on {os.path.basename(file)}: {err}")
                    
                    retry = messagebox.askretrycancel(
                        "Compression Error", 
                        f"Failed to compress:\n{os.path.basename(file)}\n\nError: {err}\n\nClick 'Retry' to try again, or 'Cancel' to skip this file."
                    )
                    if retry:
                        continue
                    else:
                        self.log(f"Skipped: {os.path.basename(file)}")
                        fail_count += 1
                        break

        self.log(f"\nFinished ECT! Success: {success_count}, Failed/Skipped: {fail_count}")
        messagebox.showinfo("Done", f"ECT compression complete.\nSuccess: {success_count}\nFailed/Skipped: {fail_count}")

    def archive_selected_files(self):
        files = filedialog.askopenfilenames(title="Select Files to Archive")
        if not files:
            return
        
        fmt = self.archive_format_var.get()
        ext = ".7z" if fmt == "7z" else ".tar.xz"
        
        out_file = filedialog.asksaveasfilename(
            title="Save Archive As",
            initialfile=f"archive{ext}",
            defaultextension=ext,
            filetypes=[(f"{fmt.upper()} Archive", f"*{ext}"), ("All Files", "*.*")]
        )
        if not out_file:
            return

        self.run_archiver_files(list(files), out_file, fmt)

    def archive_selected_directory(self):
        src_dir = filedialog.askdirectory(title="Select Directory to Archive")
        if not src_dir:
            return

        fmt = self.archive_format_var.get()
        ext = ".7z" if fmt == "7z" else ".tar.xz"
        default_name = os.path.basename(os.path.normpath(src_dir)) + ext
        
        out_file = filedialog.asksaveasfilename(
            title="Save Archive As",
            initialfile=default_name,
            defaultextension=ext,
            filetypes=[(f"{fmt.upper()} Archive", f"*{ext}"), ("All Files", "*.*")]
        )
        if not out_file:
            return

        self.run_archiver_directory(src_dir, out_file, fmt)

    def run_archiver_files(self, files, out_file, fmt):
        if fmt == "7z":
            if subprocess.run(["which", "7z"], capture_output=True).returncode != 0:
                messagebox.showerror("Error", "'7z' command not found.\nInstall it via: sudo apt install p7zip-full")
                return
            cmd = ["7z", "a", out_file] + files
        else: # tar.xz using 9e compression level
            if subprocess.run(["which", "tar"], capture_output=True).returncode != 0:
                messagebox.showerror("Error", "'tar' command not found.")
                return
            os.environ["XZ_OPT"] = "-9e"
            cmd = ["tar", "-cJf", out_file] + files

        self.log(f"Starting archive creation: {' '.join(cmd)}")
        self.update()

        res = subprocess.run(cmd, capture_output=True, text=True)
        if "XZ_OPT" in os.environ:
            del os.environ["XZ_OPT"]

        if res.returncode == 0:
            self.log(f"Successfully created archive: {out_file}")
            messagebox.showinfo("Success", f"Archive created successfully:\n{out_file}")
        else:
            err = res.stderr.strip() or res.stdout.strip()
            self.log(f"Archive error: {err}")
            messagebox.showerror("Error", f"Failed to create archive:\n{err}")

    def run_archiver_directory(self, src_dir, out_file, fmt):
        if fmt == "7z":
            if subprocess.run(["which", "7z"], capture_output=True).returncode != 0:
                messagebox.showerror("Error", "'7z' command not found.\nInstall it via: sudo apt install p7zip-full")
                return
            cmd = ["7z", "a", out_file, src_dir]
        else: # tar.xz using 9e compression level
            if subprocess.run(["which", "tar"], capture_output=True).returncode != 0:
                messagebox.showerror("Error", "'tar' command not found.")
                return
            os.environ["XZ_OPT"] = "-9e"
            parent_dir = os.path.dirname(os.path.normpath(src_dir))
            dir_name = os.path.basename(os.path.normpath(src_dir))
            cmd = ["tar", "-cJf", out_file, "-C", parent_dir, dir_name]

        self.log(f"Starting archive creation: {' '.join(cmd)}")
        self.update()

        res = subprocess.run(cmd, capture_output=True, text=True)
        if "XZ_OPT" in os.environ:
            del os.environ["XZ_OPT"]

        if res.returncode == 0:
            self.log(f"Successfully created archive: {out_file}")
            messagebox.showinfo("Success", f"Archive created successfully:\n{out_file}")
        else:
            err = res.stderr.strip() or res.stdout.strip()
            self.log(f"Archive error: {err}")
            messagebox.showerror("Error", f"Failed to create archive:\n{err}")

if __name__ == "__main__":
    app = ECTGui()
    app.mainloop()
