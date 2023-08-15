#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sqlite3.h>
#include <time.h>

#define DATABASE_NAME "text_files.db"
#define TABLE_NAME "files"

void create_database() {
    sqlite3 *db;
    char *err_msg = 0;
    int rc = sqlite3_open(DATABASE_NAME, &db);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    char *sql = "CREATE TABLE IF NOT EXISTS " TABLE_NAME " (folder_name TEXT, file_name TEXT, content TEXT, created_at TEXT, modified_at TEXT, PRIMARY KEY(folder_name, file_name));";

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        exit(1);
    }

    sqlite3_close(db);
}

void insert_or_update_database(const char *folder_name, const char *file_name, const char *content) {
    sqlite3 *db;
    char *err_msg = 0;
    int rc = sqlite3_open(DATABASE_NAME, &db);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char timestamp[64];
    sprintf(timestamp, "%04d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    char *sql = sqlite3_mprintf("INSERT OR REPLACE INTO " TABLE_NAME " (folder_name, file_name, content, created_at, modified_at) VALUES ('%q', '%q', '%q', COALESCE((SELECT created_at FROM " TABLE_NAME " WHERE folder_name = '%q' AND file_name = '%q'), '%q'), '%q');", folder_name, file_name, content, folder_name, file_name, timestamp, timestamp);

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }

    sqlite3_free(sql);
    sqlite3_close(db);
}

void scan_folder_and_import(const char *folder_path) {
    DIR *dir;
    struct dirent *entry;
    char file_path[1024];

    if ((dir = opendir(folder_path)) == NULL) {
        perror("Failed to open directory");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG && strstr(entry->d_name, ".txt")) {
            sprintf(file_path, "%s/%s", folder_path, entry->d_name);
            FILE *file = fopen(file_path, "r");
            if (file) {
                char content[4096] = {0};
                size_t bytesRead = fread(content, sizeof(char), sizeof(content) - 1, file);
                content[bytesRead] = '\0';
                fclose(file);
                insert_or_update_database(folder_path, entry->d_name, content);
            }
        }
    }

    closedir(dir);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <folder_path>\n", argv[0]);
        return 1;
    }

    create_database();
    scan_folder_and_import(argv[1]);

    return 0;
}
