/*
* Copyright (C) 2026 [Your Name or Organization]
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <https://gnu.org>.
* Andrew D. Harris <andrew2325@gmail.com> used Gemini to make this app.  
* I am not an engineer or anything.
*/


#include <tcl.h>
#include <tk.h>
#include <cstdlib>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <dirent.h>

std::vector<std::string> ParseTclList(Tcl_Interp *interp, const char *listStr) {
    int argc = 0;
    const char **argv = nullptr;
    std::vector<std::string> result;
    if (Tcl_SplitList(interp, listStr, &argc, &argv) == TCL_OK) {
        for (int i = 0; i < argc; ++i) {
            result.push_back(argv[i]);
        }
        Tcl_Free((char *)argv);
    }
    return result;
}

void LogMessage(Tcl_Interp *interp, const std::string &msg) {
    std::string cmd = ".log_frame.txt insert end \"" + msg + "\\n\"\n";
    Tcl_Eval(interp, cmd.c_str());
    Tcl_Eval(interp, ".log_frame.txt see end\nupdate");
}

void CollectFiles(const std::string &dir, std::vector<std::string> &files) {
    DIR *dp = opendir(dir.c_str());
    if (!dp) return;
    struct dirent *entry;
    while ((entry = readdir(dp)) != nullptr) {
        std::string name = entry->d_name;
        if (name == "." || name == "..") continue;
        std::string fullpath = dir + "/" + name;
        struct stat st;
        if (stat(fullpath.c_str(), &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                CollectFiles(fullpath, files);
            } else if (S_ISREG(st.st_mode)) {
                size_t dot_pos = name.find_last_of('.');
                if (dot_pos != std::string::npos) {
                    std::string ext = name.substr(dot_pos);
                    for (auto &c : ext) c = tolower(c);
                    
                    if (ext == ".png" || ext == ".jpg" || ext == ".zip" || ext == ".gz" || ext == ".jpeg") {
                        files.push_back(fullpath);
                    }
                }
            }
        }
    }
    closedir(dp);
}

int ProcessECTCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[]) {
    if (system("which ect > /dev/null 2>&1") != 0) {
        Tcl_Eval(interp, "tk_messageBox -icon error -title \"Error\" -message \"'ect' command not found.\\nInstall it via source or package manager.\"");
        return TCL_OK;
    }

    const char *mode = argv[1];
    std::vector<std::string> files_to_process;

    if (std::string(mode) == "files") {
        Tcl_Eval(interp, "tk_getOpenFile -multiple 1 -title \"Select Files to Optimize\" -filetypes {{\"Supported Files\" {*.png *.jpg *.jpeg *.zip *.gz}} {\"All Files\" {*.*}}}");
        std::string resStr = Tcl_GetString(Tcl_GetObjResult(interp));
        if (resStr.empty()) return TCL_OK;
        files_to_process = ParseTclList(interp, resStr.c_str());
    } else {
        Tcl_Eval(interp, "tk_chooseDirectory -title \"Select Directory to Optimize\"");
        std::string dirStr = Tcl_GetString(Tcl_GetObjResult(interp));
        if (dirStr.empty()) return TCL_OK;
        CollectFiles(dirStr, files_to_process);
        if (files_to_process.empty()) {
            Tcl_Eval(interp, "tk_messageBox -icon info -title \"No Files\" -message \"No supported files found in selected directory.\"");
            return TCL_OK;
        }
    }

    Tcl_Eval(interp, "set _lvl [.container.ect.opt.spn get]");
    std::string level = "-" + std::string(Tcl_GetString(Tcl_GetObjResult(interp)));
    
    Tcl_Eval(interp, "set _strip $strip_var");
    bool strip = (std::string(Tcl_GetString(Tcl_GetObjResult(interp))) == "1");

    LogMessage(interp, "Starting ECT compression for " + std::to_string(files_to_process.size()) + " items at level " + level + "...");

    int success_count = 0;
    int fail_count = 0;

    for (const auto &file : files_to_process) {
        std::string cmd = "ect " + level;
        if (strip) cmd += " -strip";
        cmd += " \"" + file + "\"";

        while (true) {
            LogMessage(interp, "Executing: " + cmd);
            int ret = system(cmd.c_str());
            
            size_t idx = file.find_last_of("/\\");
            std::string basename = (idx == std::string::npos) ? file : file.substr(idx + 1);

            if (ret == 0) {
                LogMessage(interp, "Success: " + basename);
                success_count++;
                break;
            } else {
                LogMessage(interp, "Error on " + basename);
                std::string box_cmd = "tk_messageBox -icon error -type retrycancel -title \"Compression Error\" -message \"Failed to compress:\\n" + basename + "\\n\\nClick 'Retry' to try again, or 'Cancel' to skip.\"";
                Tcl_Eval(interp, box_cmd.c_str());
                std::string choice = Tcl_GetString(Tcl_GetObjResult(interp));
                
                if (choice == "retry") {
                    continue;
                } else {
                    LogMessage(interp, "Skipped: " + basename);
                    fail_count++;
                    break;
                }
            }
        }
    }

    LogMessage(interp, "\nFinished ECT! Success: " + std::to_string(success_count) + ", Failed/Skipped: " + std::to_string(fail_count));
    Tcl_Eval(interp, ("tk_messageBox -icon info -title \"Done\" -message \"ECT compression complete.\\nSuccess: " + std::to_string(success_count) + "\\nFailed/Skipped: " + std::to_string(fail_count) + "\"").c_str());

    return TCL_OK;
}

int ProcessArchiveCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[]) {
    const char *mode = argv[1];
    
    Tcl_Eval(interp, "set _fmt $archive_format_var");
    std::string fmt = Tcl_GetString(Tcl_GetObjResult(interp));
    std::string ext = (fmt == "7z") ? ".7z" : ".tar.xz";

    std::vector<std::string> targets;
    std::string default_name = "archive" + ext;

    if (std::string(mode) == "files") {
        Tcl_Eval(interp, "tk_getOpenFile -multiple 1 -title \"Select Files to Archive\"");
        std::string resStr = Tcl_GetString(Tcl_GetObjResult(interp));
        if (resStr.empty()) return TCL_OK;
        targets = ParseTclList(interp, resStr.c_str());
    } else {
        Tcl_Eval(interp, "tk_chooseDirectory -title \"Select Directory to Archive\"");
        std::string dirStr = Tcl_GetString(Tcl_GetObjResult(interp));
        if (dirStr.empty()) return TCL_OK;
        targets.push_back(dirStr);
        
        size_t idx = dirStr.find_last_of("/\\");
        std::string dirname = (idx == std::string::npos) ? dirStr : dirStr.substr(idx + 1);
        default_name = dirname + ext;
    }

    std::string dialog_cmd = "tk_getSaveFile -title \"Save Archive As\" -initialfile \"" + default_name + "\" -defaultextension " + ext + " -filetypes {{\"Archive\" {*" + ext + "}} {\"All Files\" {*.*}}}";
    if (Tcl_Eval(interp, dialog_cmd.c_str()) == TCL_ERROR) return TCL_OK;
    
    std::string out_file = Tcl_GetString(Tcl_GetObjResult(interp));
    if (out_file.empty()) return TCL_OK;

    std::string cmd = "";
    if (fmt == "7z") {
        if (system("which 7z > /dev/null 2>&1") != 0) {
            Tcl_Eval(interp, "tk_messageBox -icon error -title \"Error\" -message \"'7z' command not found.\\nInstall it via: sudo apt install p7zip-full\"");
            return TCL_OK;
        }
        cmd = "7z a \"" + out_file + "\"";
        for (const auto &t : targets) cmd += " \"" + t + "\"";
    } else {
        if (system("which tar > /dev/null 2>&1") != 0) {
            Tcl_Eval(interp, "tk_messageBox -icon error -title \"Error\" -message \"'tar' command not found.\"");
            return TCL_OK;
        }
        if (std::string(mode) == "dir") {
            std::string src = targets[0];
            size_t idx = src.find_last_of("/\\");
            std::string parent = (idx == std::string::npos) ? "." : src.substr(0, idx);
            std::string name = (idx == std::string::npos) ? src : src.substr(idx + 1);
            cmd = "XZ_OPT=-9e tar -cJf \"" + out_file + "\" -C \"" + parent + "\" \"" + name + "\"";
        } else {
            cmd = "XZ_OPT=-9e tar -cJf \"" + out_file + "\"";
            for (const auto &t : targets) cmd += " \"" + t + "\"";
        }
    }

    LogMessage(interp, "Starting archive creation: " + cmd);
    int ret = system(cmd.c_str());

    if (ret == 0) {
        LogMessage(interp, "Successfully created archive: " + out_file);
        Tcl_Eval(interp, ("tk_messageBox -icon info -title \"Success\" -message \"Archive created successfully:\\n" + out_file + "\"").c_str());
    } else {
        LogMessage(interp, "Archive error encountered.");
        Tcl_Eval(interp, "tk_messageBox -icon error -title \"Error\" -message \"Failed to create archive.\"");
    }

    return TCL_OK;
}

int main(int argc, char **argv) {
    Tcl_FindExecutable(argv[0]);
    Tcl_Interp *interp = Tcl_CreateInterp();

    if (Tcl_Init(interp) == TCL_ERROR) return 1;
    if (Tk_Init(interp) == TCL_ERROR) return 1;

    Tcl_CreateCommand(interp, "CppProcessECT", ProcessECTCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateCommand(interp, "CppProcessArchive", ProcessArchiveCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);

    std::string ui_script = 
        "wm title . \"cetera\"\n"
        "wm geometry . \"560x520\"\n"
        "wm resizable . 0 0\n"
        "option add *background #1e1e1e\n"
        "option add *foreground #ffffff\n"
        "option add *Label*background #1e1e1e\n"
        "option add *Label*foreground #ffffff\n"
        "option add *Labelframe*background #1e1e1e\n"
        "option add *Labelframe*foreground #cccccc\n"
        "option add *Radiobutton*background #1e1e1e\n"
        "option add *Radiobutton*foreground #ffffff\n"
        "option add *Radiobutton*selectColor #333333\n"
        "option add *Frame*background #1e1e1e\n"
        ". configure -background #1e1e1e\n"
        "label .title -text \"cetera - ECT & Archive Tool\" -font {Helvetica 13 bold}\n"
        "pack .title -pady 12\n"
        "frame .container\n"
        "pack .container -pady 5 -fill x -padx 20\n"
        "frame .container.ect\n"
        "frame .container.ect.opt\n"
        "pack .container.ect.opt -pady 5 -fill x\n"
        "label .container.ect.opt.lbl -text \"ECT Level (1-9):\" -fg #cccccc\n"
        "spinbox .container.ect.opt.spn -from 1 -to 9 -width 4\n"
        ".container.ect.opt.spn delete 0 end\n"
        ".container.ect.opt.spn insert 0 9\n"
        "pack .container.ect.opt.lbl -side left -padx 5\n"
        "pack .container.ect.opt.spn -side left -padx 5\n"
        "global strip_var\n"
        "set strip_var 1\n"
        "checkbutton .container.ect.opt.chk -text \"Strip Metadata\" -variable strip_var -selectcolor #333333 -activebackground #1e1e1e -activeforeground #ffffff\n"
        "pack .container.ect.opt.chk -side right -padx 5\n"
        "frame .container.ect.btns\n"
        "pack .container.ect.btns -pady 10\n"
        "button .container.ect.btns.file -text \"Select Files (ECT)\" -command {CppProcessECT files} -bg #007acc -fg white -padx 8 -pady 6 -relief flat -font {Helvetica 9 bold}\n"
        "button .container.ect.btns.dir -text \"Select Directory (ECT)\" -command {CppProcessECT dir} -bg #28a745 -fg white -padx 8 -pady 6 -relief flat -font {Helvetica 9 bold}\n"
        "button .container.ect.btns.switch -text \"Archive Menu...\" -command ShowArchiveView -bg #d97706 -fg white -padx 8 -pady 6 -relief flat -font {Helvetica 9 bold}\n"
        "pack .container.ect.btns.file -side left -padx 4\n"
        "pack .container.ect.btns.dir -side left -padx 4\n"
        "pack .container.ect.btns.switch -side left -padx 4\n"
        "frame .container.arc\n"
        "label .container.arc.lbl -text \"Archive Creator\" -font {Helvetica 11 bold}\n"
        "pack .container.arc.lbl -pady 5\n"
        "frame .container.arc.fmt\n"
        "pack .container.arc.fmt -pady 5\n"
        "label .container.arc.fmt.lbl -text \"Format:\" -fg #cccccc\n"
        "global archive_format_var\n"
        "set archive_format_var \"7z\"\n"
        "radiobutton .container.arc.fmt.r1 -text \"p7zip (.7z)\" -variable archive_format_var -value \"7z\" -selectcolor #333333 -activebackground #1e1e1e -activeforeground #ffffff\n"
        "radiobutton .container.arc.fmt.r2 -text \"tar.xz (.tar.xz)\" -variable archive_format_var -value \"tar.xz\" -selectcolor #333333 -activebackground #1e1e1e -activeforeground #ffffff\n"
        "pack .container.arc.fmt.lbl -side left -padx 5\n"
        "pack .container.arc.fmt.r1 -side left -padx 5\n"
        "pack .container.arc.fmt.r2 -side left -padx 5\n"
        "frame .container.arc.btns\n"
        "pack .container.arc.btns -pady 10\n"
        "button .container.arc.btns.farc -text \"Select Files to Archive\" -command {CppProcessArchive files} -bg #007acc -fg white -padx 10 -pady 6 -relief flat -font {Helvetica 9 bold}\n"
        "button .container.arc.btns.darc -text \"Select Directory to Archive\" -command {CppProcessArchive dir} -bg #28a745 -fg white -padx 10 -pady 6 -relief flat -font {Helvetica 9 bold}\n"
        "button .container.arc.btns.back -text \"< Back\" -command ShowECTView -bg #555555 -fg white -padx 10 -pady 6 -relief flat -font {Helvetica 9 bold}\n"
        "pack .container.arc.btns.farc -side left -padx 6\n"
        "pack .container.arc.btns.darc -side left -padx 6\n"
        "pack .container.arc.btns.back -side left -padx 6\n"
        "frame .log_frame\n"
        "pack .log_frame -pady 10 -fill both -expand 1 -padx 20\n"
        "text .log_frame.txt -bg #2d2d2d -fg #ffffff -insertbackground white -relief flat -font {Courier 9} -yscrollcommand {.log_frame.scr set}\n"
        "scrollbar .log_frame.scr -command {.log_frame.txt yview}\n"
        "pack .log_frame.txt -side left -fill both -expand 1\n"
        "pack .log_frame.scr -side right -fill y\n"
        "proc ShowECTView {} {\n"
        "    pack forget .container.arc\n"
        "    pack .container.ect -fill x -expand 1\n"
        "}\n"
        "proc ShowArchiveView {} {\n"
        "    pack forget .container.ect\n"
        "    pack .container.arc -fill x -expand 1\n"
        "}\n"
        "ShowECTView\n";

    if (Tcl_Eval(interp, ui_script.c_str()) == TCL_ERROR) {
        const char *err = Tcl_GetStringResult(interp);
        fprintf(stderr, "Tcl Error: %s\n", err);
        return 1;
    }

    Tk_MainLoop();
    return 0;
}
