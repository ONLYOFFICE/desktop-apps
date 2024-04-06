/*
 * (c) Copyright Ascensio System SIA 2010-2019
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation. In accordance with
 * Section 7(a) of the GNU AGPL its Section 15 shall be amended to the effect
 * that Ascensio System SIA expressly excludes the warranty of non-infringement
 * of any third-party rights.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR  PURPOSE. For
 * details, see the GNU AGPL at: http://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA at 20A-12 Ernesta Birznieka-Upisha
 * street, Riga, Latvia, EU, LV-1050.
 *
 * The  interactive user interfaces in modified source and object code versions
 * of the Program must display Appropriate Legal Notices, as required under
 * Section 5 of the GNU AGPL version 3.
 *
 * Pursuant to Section 7(b) of the License you must retain the original Product
 * logo when distributing the program. Pursuant to Section 7(e) we decline to
 * grant you any rights under trademark law for use of our trademarks.
 *
 * All the Product's GUI elements, including illustrations and icon sets, as
 * well as technical writing content are licensed under the terms of the
 * Creative Commons Attribution-ShareAlike 4.0 International. See the License
 * terms at http://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
*/

#include "platform_linux/utils.h"
#include "version.h"
#include <fstream>
#include <stack>
#include <algorithm>
#include <sys/stat.h>
#include <gtk/gtk.h>
#include <string.h>
//#include <openssl/md5.h>
#include <fcntl.h>
#include "../../src/defines.h"
#include "../../src/prop/defines_p.h"

#define APP_CONFIG_PATH "/.config/" REG_GROUP_KEY "/" REG_APP_NAME ".conf"
//#define BUFSIZE 1024


static void replace(string &str, const string &from, const string &to)
{
    size_t pos = 0;
    while ((pos = str.find(from, pos)) != string::npos) {
        str.replace(pos, from.length(), to);
        pos += to.length();
    }
}

static bool moving_through_copy(const string &oldFile, const string &newFile)
{
    struct stat st;
    if (stat(oldFile.c_str(), &st) != 0)
        return false;

    char buf[4096];
    int fd_src = -1, fd_dst = -1, n_read = 0;
    fd_src = open(oldFile.c_str(), O_RDONLY);
    if (fd_src < 0)
        return false;

    fd_dst = open(newFile.c_str(), O_WRONLY | O_CREAT, st.st_mode);
    if (fd_dst < 0)
        return false;

    while ((n_read = read(fd_src, buf, sizeof(buf))) > 0) {
        if (write(fd_dst, buf, n_read) != n_read) {
            close(fd_src);
            close(fd_dst);
            return false;
        }
    }
    if (close(fd_src) != 0 || close(fd_dst) != 0)
        return false;

    if (unlink(oldFile.c_str()) != 0)
        return false;
    return true;
};

static bool moving_folder_content(const string &from, const string &to, bool use_rename)
{
    list<string> filesList;
    string error;
    if (!NS_File::GetFilesList(from, &filesList, error, false)) {
        NS_Logger::WriteLog(DEFAULT_ERROR_MESSAGE + " "+ error);
        return false;
    }

    const size_t sourceLength = from.length();
    for (const string &sourcePath : filesList) {
        if (!sourcePath.empty()) {
            string dest = to + sourcePath.substr(sourceLength);
            if (!NS_File::dirExists(NS_File::parentPath(dest)) && !NS_File::makePath(NS_File::parentPath(dest))) {
                NS_Logger::WriteLog("Can't create path: " + NS_File::parentPath(dest));
                return false;
            }
            if (use_rename) {
                if (rename(sourcePath.c_str(), dest.c_str()) != 0) {
                    NS_Logger::WriteLog("Can't move file from " + sourcePath + " to " + dest + ". " + NS_Utils::GetLastErrorAsString());
                    return false;
                }
            } else {
                if (!moving_through_copy(sourcePath, dest)) {
                    NS_Logger::WriteLog("Can't move file from " + sourcePath + " to " + dest + ". " + NS_Utils::GetLastErrorAsString());
                    return false;
                }
            }
        }
    }
    return true;
}

namespace NS_Utils
{
    string GetLastErrorAsString()
    {        
        char buff[LINE_MAX] = {0};
        char *res = strerror_r(errno, buff, sizeof(buff));
        return res ? string(res) : "";
    }

    int ShowMessage(string str, bool showError)
    {
        if (showError)
            str += " " + GetLastErrorAsString();

        gtk_init(NULL, NULL);
        GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                      "%s", str.c_str());
        string prod_name = _TR(VER_PRODUCTNAME_STR);
        gtk_window_set_title(GTK_WINDOW(dialog), prod_name.c_str());
        int res = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        while (gtk_events_pending())
            gtk_main_iteration_do(FALSE);
        return res;
    }

//    string GetSysLanguage()
//    {
//        string lang("en_EN");
//        size_t pos = std::string::npos;
//        if (char *_lang = getenv("LANG")) {
//            lang = _lang;
//            pos = lang.find('.');
//        }
//        return (pos == std::string::npos) ? lang : lang.substr(0, pos);
//    }

    string GetAppLanguage()
    {
        string lang = "en_US", value = "locale=", path = string("/home/") + getlogin() + APP_CONFIG_PATH;
        list<string> lst;
        NS_File::readFile(path, lst);
        for (const auto &str : lst) {
            size_t pos = str.find(value);
            if (pos != string::npos) {
                lang = str.substr(pos + value.length());
                std::replace(lang.begin(), lang.end(), '-', '_');
                return lang;
            }
        }
        return lang;
    }
}

namespace NS_File
{
    bool GetFilesList(const string &path, list<string> *lst, string &error, bool ignore_locked, bool folders_only)
    {
        DIR *dir = opendir(path.c_str());
        if (!dir) {
            error = string("FindFirstFile invalid handle value: ") + path;
            return false;
        }

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            char _path[PATH_MAX];
            snprintf(_path, sizeof(_path), "%s/%s", path.c_str(), entry->d_name);
            struct stat info;
            if (stat(_path, &info) != 0) {
                error = string("Error getting file information: ") + _path;
                closedir(dir);
                return false;
            }

            if (S_ISDIR(info.st_mode)) {
                if (ignore_locked && access(_path, R_OK) != 0)
                    continue;
                if (folders_only) {
                    lst->push_back(string("/") + entry->d_name);
                    continue;
                }
                if (!GetFilesList(_path, lst, error, ignore_locked)) {
                    closedir(dir);
                    return false;
                }
            } else
            if (!folders_only && S_ISREG(info.st_mode))
                lst->push_back(_path);

        }
        closedir(dir);
        return true;
    }

    bool readFile(const string &filePath, list<string> &linesList)
    {
        std::ifstream file(filePath.c_str(), std::ios_base::in);
        if (!file.is_open()) {
            NS_Logger::WriteLog("An error occurred while opening: " + filePath);
            return false;
        }
        string line;
        while (std::getline(file, line))
            linesList.push_back(line);

        file.close();
        return true;
    }

    bool writeToFile(const string &filePath, list<string> &linesList)
    {
        std::ofstream file(filePath.c_str(), std::ios_base::out);
        if (!file.is_open()) {
            NS_Logger::WriteLog("An error occurred while writing: " + filePath);
            return false;
        }
        for (auto &line : linesList)
            file << line << std::endl;

        file.close();
        return true;
    }

    bool runProcess(const string &fileName, const string &args)
    {
        string src(fileName);
        replace(src, " ", "\\ ");
        string cmd = ("LD_LIBRARY_PATH=" + parentPath(src) + " " + src + " " + args + " &");
        return system(cmd.c_str()) == 0 ? true: false;
    }

    bool isProcessRunning(const string &fileName)
    {      
        DIR *proc_dir = opendir("/proc");
        if (!proc_dir)
            return false;

        struct dirent* entry;
        while ((entry = readdir(proc_dir)) != NULL) {
            if (entry->d_type == DT_DIR && strtol(entry->d_name, NULL, 10) > 0) {
                char cmd_file[256];
                snprintf(cmd_file, sizeof(cmd_file), "/proc/%s/cmdline", entry->d_name);

                FILE* cmd_file_ptr = fopen(cmd_file, "r");
                if (!cmd_file_ptr)
                    continue;

                char cmd_line[256];
                fgets(cmd_line, sizeof(cmd_line), cmd_file_ptr);
                fclose(cmd_file_ptr);

                if (strstr(basename(cmd_line), fileName.c_str()) != NULL) {
                    closedir(proc_dir);
                    return true;
                }
            }
        }

        closedir(proc_dir);
        return false;
    }

    bool fileExists(const string &filePath)
    {
        struct stat st;
        if (stat(filePath.c_str(), &st) != 0)
            return false;
        return S_ISREG(st.st_mode);
    }

    bool dirExists(const string &dirName) {
        struct stat st;
        if (stat(dirName.c_str(), &st) != 0)
            return false;
        return S_ISDIR(st.st_mode);
    }

    bool dirIsEmpty(const string &dirName)
    {
        DIR *dir = opendir(dirName.c_str());
        if (!dir)
            return true;

        int count = 0;
        struct dirent *entry;
        while (count == 0 && (entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
                count++;
        }
        closedir(dir);
        return (count == 0);
    }

    bool makePath(const string &path)
    {
        std::stack<string> pathsList;
        string last_path(path);
        while (!last_path.empty() && !dirExists(last_path)) {
            pathsList.push(last_path);
            last_path = parentPath(last_path);
        }
        while(!pathsList.empty()) {
            if (mkdir(pathsList.top().c_str(), S_IRWXU | S_IRWXG | S_IRWXO) != 0)
                return false;
            pathsList.pop();
        }
        return true;
    }

    bool replaceFile(const string &oldFilePath, const string &newFilePath)
    {
        struct stat src, dst;
        if (stat(oldFilePath.c_str(), &src) != 0)
            return false;
        if (!S_ISREG(src.st_mode))
            return false;
        if (stat(parentPath(newFilePath).c_str(), &dst) != 0)
            return false;
        if (src.st_dev == dst.st_dev) {
            if (rename(oldFilePath.c_str(), newFilePath.c_str()) != 0)
                return false;
        } else {
            if (!moving_through_copy(oldFilePath, newFilePath))
                return false;
        }
        return true;
    }

    bool replaceFolder(const string &from, const string &to, bool remove_existing)
    {
        struct stat src, dst;
        if (stat(from.c_str(), &src) != 0)
            return false;
        if (stat(parentPath(to).c_str(), &dst) != 0)
            return false;
        if (!S_ISDIR(src.st_mode) || !S_ISDIR(dst.st_mode))
            return false;

        if(remove_existing && !dirIsEmpty(to) && !removeDirRecursively(to))
            return false;

        bool can_use_rename = (src.st_dev == dst.st_dev);
        if (!dirExists(to) || dirIsEmpty(to)) {
            if (can_use_rename) {
                if (rename(from.c_str(), to.c_str()) != 0)
                    return false;
            } else {
                if (!moving_folder_content(from, to, false))
                    return false;
            }
        } else {
            if (!moving_folder_content(from, to, can_use_rename))
                return false;
        }
        removeDirRecursively(from);
        return true;
    }

    bool removeFile(const string &filePath)
    {
        return (remove(filePath.c_str()) == 0) ? true: false;
    }

    bool removeDirRecursively(const string &dir)
    {
        DIR *_dir = opendir(dir.c_str());
        if (!_dir)
            return false;

        struct dirent *entry;
        while ((entry = readdir(_dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            char path[PATH_MAX] = {0};
            snprintf(path, sizeof(path), "%s/%s", dir.c_str(), entry->d_name);
            struct stat info;
            if (stat(path, &info) != 0) {
                closedir(_dir);
                return false;
            }

            if (S_ISDIR(info.st_mode)) {
                if (!removeDirRecursively(path)) {
                    closedir(_dir);
                    return false;
                }
            } else {
                if (remove(path) != 0) {
                    closedir(_dir);
                    return false;
                }
            }
        }

        closedir(_dir);
        if (rmdir(dir.c_str()) != 0)
            return false;
        return true;
    }

    string parentPath(const string &path)
    {
        string::size_type delim = path.find_last_of("\\/");
        return (delim == string::npos) ? "" : path.substr(0, delim);
    }

    string tempPath()
    {
        const char *path = getenv("TMP");
        if (!path)
            path = getenv("TEMP");
        if (!path)
            path = getenv("TMPDIR");
        if (!path)
            path = "/tmp";
        return string(path);
    }

    string appPath()
    {
        char path[PATH_MAX] = {0};
        ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
        return (count > 0) ? parentPath(string(path, count)) : "";
    }

//    string getFileHash(const string &fileName)
//    {
//        FILE *file = fopen(fileName.c_str(), "rb");
//        if (!file)
//            return "";

//        int bytes;
//        unsigned char data[1024];
//        unsigned char digest[MD5_DIGEST_LENGTH];
//        MD5_CTX mdContext;
//        MD5_Init(&mdContext);
//        while ((bytes = fread(data, 1, 1024, file)) != 0)
//            MD5_Update(&mdContext, data, bytes);

//        MD5_Final(digest, &mdContext);
//        fclose(file);

//        std::ostringstream oss;
//        for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
//            oss.fill('0');
//            oss.width(2);
//            oss << std::hex << static_cast<const int>(digest[i]);
//        }
//        return oss.str();
//    }

//    bool verifyEmbeddedSignature(const string &fileName)
//    {
//        return false;
//    }
}

namespace NS_Logger
{
    bool allow_write_log = false;

    void AllowWriteLog()
    {
        allow_write_log = true;
    }

    void WriteLog(const string &log, bool showMessage)
    {
        if (allow_write_log) {
            string filpPath(NS_File::appPath() + "/service_log.txt");
            std::ofstream file(filpPath.c_str(), std::ios::app);
            if (!file.is_open()) {
                return;
            }
            file << log << std::endl;
            file.close();
        }
        if (showMessage)
            NS_Utils::ShowMessage(log);
    }
}
