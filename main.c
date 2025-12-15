//compile command:
// gcc main.c sqlite3.c controller.c -o main.exe -I. -Wall

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include "sqlite3.h"
#include "controller.h"

volatile sig_atomic_t running = 1;

void signal_handler(int sig) {
    if (sig == SIGINT) {
        running = 0;
        printf("\nShutting down...\n");
    }
}

typedef struct {
    sqlite3* conn;
} db;

//initialize database if doesn't exist
int db_init(db* d, const char* filename) {
    if (sqlite3_open(filename, &d->conn) != SQLITE_OK) {
        return 0;
    }

    const char* sql_create =
        "CREATE TABLE IF NOT EXISTS button_log ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "button TEXT NOT NULL, "
        "timestamp INTEGER NOT NULL);";

    char* errmsg = NULL;
    if (sqlite3_exec(d->conn, sql_create, 0, 0, &errmsg) != SQLITE_OK) {
        sqlite3_free(errmsg);
        return 0;
    }

    return 1;
}

void db_close(db* d) {
    sqlite3_close(d->conn);
}

//log a button press with timestamp
int db_log_button(db* d, const char* button, time_t timestamp) {
    const char* sql_insert =
        "INSERT INTO button_log (button, timestamp) VALUES (?, ?);";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(d->conn, sql_insert, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }

    sqlite3_bind_text(stmt, 1, button, -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 2, (sqlite3_int64)timestamp);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return 0;
    }

    sqlite3_finalize(stmt);
    return 1;
}

//print all button presses from database
void db_print(db* d) {
    const char* sql_select = "SELECT button, timestamp FROM button_log ORDER BY id;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(d->conn, sql_select, -1, &stmt, NULL) != SQLITE_OK) {
        return;
    }

    printf("\n=== Button Press Log ===");
    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* button = sqlite3_column_text(stmt, 0);
        sqlite3_int64 timestamp = sqlite3_column_int64(stmt, 1);
        printf("\n[%lld] %s pressed", timestamp, button);
        count++;
    }
    printf("\n\nTotal presses: %d\n", count);

    sqlite3_finalize(stmt);
}

//clear all button presses from database
int db_clear(db* d) {
    const char* sql_delete = "DELETE FROM button_log;";
    char* errmsg = NULL;
    
    if (sqlite3_exec(d->conn, sql_delete, 0, 0, &errmsg) != SQLITE_OK) {
        fprintf(stderr, "Failed to clear database: %s\n", errmsg);
        sqlite3_free(errmsg);
        return 0;
    }
    
    printf("Database cleared successfully.\n");
    return 1;
}

int main() {
    db data;

    if (!db_init(&data, "mydb.sqlite")) {
        return 1;
    }

    printf("=== Button Logger Menu ===\n");
    printf("1. Start logging button presses\n");
    printf("2. Read existing button presses\n");
    printf("3. Clear all button presses\n");
    printf("\nEnter choice (1-3): ");
    
    int choice;
    if (scanf("%d", &choice) != 1) {
        fprintf(stderr, "Invalid input\n");
        db_close(&data);
        return 1;
    }

    //read db
    if (choice == 2) {
        db_print(&data);
        db_close(&data);
        return 0;
    }
    //clear db 
    else if (choice == 3) {
        db_clear(&data);
        db_close(&data);
        return 0;
    } else if (choice != 1) {
        fprintf(stderr, "Invalid choice\n");
        db_close(&data);
        return 1;
    }

    //initialize
    if (!gpio_init()) {
        fprintf(stderr, "Failed to initialize USB controller.\n");
        db_close(&data);
        return 1;
    }

    signal(SIGINT, signal_handler);

    printf("Controller initialized. Logging button presses...\n");
    printf("Press Ctrl+C to stop and save.\n\n");

    uint16_t last_state = read_controller(); 
    printf("Initial controller state: 0x%04x\n\n", last_state);

    //read input until sigint
    while (running) {
        uint16_t current_state = read_controller();
        time_t current_time = time(NULL);

        if (button_pressed(current_state, BTN_A) && !button_pressed(last_state, BTN_A)) {
            printf("[%lld] A pressed\n", (long long)current_time);
            db_log_button(&data, "A", current_time);
        }
        if (button_pressed(current_state, BTN_B) && !button_pressed(last_state, BTN_B)) {
            printf("[%lld] B pressed\n", (long long)current_time);
            db_log_button(&data, "B", current_time);
        }
        if (button_pressed(current_state, BTN_X) && !button_pressed(last_state, BTN_X)) {
            printf("[%lld] X pressed\n", (long long)current_time);
            db_log_button(&data, "X", current_time);
        }
        if (button_pressed(current_state, BTN_Y) && !button_pressed(last_state, BTN_Y)) {
            printf("[%lld] Y pressed\n", (long long)current_time);
            db_log_button(&data, "Y", current_time);
        }
        if (button_pressed(current_state, BTN_START) && !button_pressed(last_state, BTN_START)) {
            printf("[%lld] START pressed\n", (long long)current_time);
            db_log_button(&data, "START", current_time);
        }
        if (button_pressed(current_state, BTN_SELECT) && !button_pressed(last_state, BTN_SELECT)) {
            printf("[%lld] SELECT pressed\n", (long long)current_time);
            db_log_button(&data, "SELECT", current_time);
        }
        if (button_pressed(current_state, BTN_L) && !button_pressed(last_state, BTN_L)) {
            printf("[%lld] L pressed\n", (long long)current_time);
            db_log_button(&data, "L", current_time);
        }
        if (button_pressed(current_state, BTN_R) && !button_pressed(last_state, BTN_R)) {
            printf("[%lld] R pressed\n", (long long)current_time);
            db_log_button(&data, "R", current_time);
        }

        last_state = current_state;
        usleep(20000);
    }

    db_print(&data);
    db_close(&data);
    return 0;
}