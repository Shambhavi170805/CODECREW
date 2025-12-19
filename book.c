#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TABLE_SIZE 101       
#define NAME_LEN 50
#define SUBJECT_LEN 50


typedef struct TimeSlot {
    int start_time;             
    int end_time;               
    char faculty[NAME_LEN];
    char subject[SUBJECT_LEN];
    struct TimeSlot *next;
} TimeSlot;


typedef struct Classroom {
    int id;                     
    int status;                 
    TimeSlot *slots;            
    struct Classroom *next;     
} Classroom;


Classroom *hash_table[TABLE_SIZE] = { NULL };



unsigned int hash_fn(int class_id) {
    return (unsigned int)(class_id % TABLE_SIZE);
}


Classroom *create_classroom(int id) {
    Classroom *room = (Classroom *)malloc(sizeof(Classroom));
    if (!room) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    room->id = id;
    room->status = 0;
    room->slots = NULL;
    room->next = NULL;
    return room;
}


Classroom *find_classroom(int id) {
    unsigned int index = hash_fn(id);
    Classroom *cur = hash_table[index];
    while (cur) {
        if (cur->id == id)
            return cur;
        cur = cur->next;
    }
    return NULL;
}


void add_classroom(int id) {
    if (find_classroom(id)) return;
    unsigned int index = hash_fn(id);
    Classroom *new_room = create_classroom(id);
    new_room->next = hash_table[index];
    hash_table[index] = new_room;
}


void init_classrooms() {
    add_classroom(101);
    add_classroom(102);
    add_classroom(103);
}


int is_conflict(TimeSlot *head, int start, int end) {
    TimeSlot *cur = head;
    while (cur) {
        if (!(end <= cur->start_time || start >= cur->end_time)) {
            return 1; 
        }
        cur = cur->next;
    }
    return 0; 
}



int book_classroom(int id, char *faculty, char *subject, int start, int end) {
    if (start >= end) return -2;
    Classroom *room = find_classroom(id);
    if (!room) return -1;

    
    if (is_conflict(room->slots, start, end)) return -3;

    
    TimeSlot *new_slot = (TimeSlot *)malloc(sizeof(TimeSlot));
    new_slot->start_time = start;
    new_slot->end_time = end;
    strncpy(new_slot->faculty, faculty, NAME_LEN - 1);
    strncpy(new_slot->subject, subject, SUBJECT_LEN - 1);
    new_slot->faculty[NAME_LEN - 1] = '\0';
    new_slot->subject[SUBJECT_LEN - 1] = '\0';
    new_slot->next = NULL;

    
    if (!room->slots || start < room->slots->start_time) {
        new_slot->next = room->slots;
        room->slots = new_slot;
    } else {
        TimeSlot *prev = room->slots;
        while (prev->next && prev->next->start_time < start)
            prev = prev->next;
        new_slot->next = prev->next;
        prev->next = new_slot;
    }

    room->status = 1; 
    return 1;
}


int vacate_classroom(int id, int start, int end) {
    Classroom *room = find_classroom(id);
    if (!room) return -1;

    TimeSlot *cur = room->slots;
    TimeSlot *prev = NULL;

    while (cur) {
        if (cur->start_time == start && cur->end_time == end) {
            if (prev) prev->next = cur->next;
            else room->slots = cur->next;
            free(cur);
            if (!room->slots) room->status = 0;
            return 1;
        }
        prev = cur;
        cur = cur->next;
    }
    return -2;
}


void display_classroom(int id) {
    Classroom *room = find_classroom(id);
    if (!room) {
        printf("Classroom %d not found.\n", id);
        return;
    }
    printf("\nClassroom %d | Status: %s\n", id, room->status ? "Occupied" : "Vacant");
    if (!room->slots) {
        printf("  No scheduled classes.\n");
        return;
    }
    TimeSlot *cur = room->slots;
    while (cur) {
        int sh = cur->start_time / 100, sm = cur->start_time % 100;
        int eh = cur->end_time / 100, em = cur->end_time % 100;
        printf("  %02d:%02d - %02d:%02d | Faculty: %-15s | Subject: %s\n",
               sh, sm, eh, em, cur->faculty, cur->subject);
        cur = cur->next;
    }
}


void display_timetable() {
    printf("\n------ Current Timetable ------\n");
    int any = 0;
    for (int i = 0; i < TABLE_SIZE; ++i) {
        Classroom *cur = hash_table[i];
        while (cur) {
            display_classroom(cur->id);
            cur = cur->next;
            any = 1;
        }
    }
    if (!any) printf("No classrooms available.\n");
    printf("-------------------------------\n");
}


void free_all() {
    for (int i = 0; i < TABLE_SIZE; ++i) {
        Classroom *cur = hash_table[i];
        while (cur) {
            TimeSlot *slot = cur->slots;
            while (slot) {
                TimeSlot *tmp = slot;
                slot = slot->next;
                free(tmp);
            }
            Classroom *tmpc = cur;
            cur = cur->next;
            free(tmpc);
        }
        hash_table[i] = NULL;
    }
}



int admin_login() {
    char id[50], pass[50];
    printf("\nEnter Admin ID: ");
    fgets(id, sizeof(id), stdin);
    id[strcspn(id, "\n")] = '\0';
    printf("Enter Password: ");
    fgets(pass, sizeof(pass), stdin);
    pass[strcspn(pass, "\n")] = '\0';

    if (strcmp(id, "test") == 0 && strcmp(pass, "1234") == 0) {
        printf("Login Successful!\n");
        return 1;
    } else {
        printf("Invalid Credentials.\n");
        return 0;
    }
}



int read_int(const char *prompt) {
    char buf[100];
    int val;
    while (1) {
        printf("%s", prompt);
        if (!fgets(buf, sizeof(buf), stdin)) return 0;
        if (sscanf(buf, "%d", &val) == 1) return val;
        printf("Invalid input. Try again.\n");
    }
}

void read_str(const char *prompt, char *out, size_t size) {
    printf("%s", prompt);
    if (fgets(out, size, stdin)) {
        out[strcspn(out, "\n")] = '\0';
    } else out[0] = '\0';
}


int main() {
    init_classrooms();

    int logged_in = 0;
    int choice;

    while (1) {
        printf("\n===== TrackMyClass: Admin Portal =====\n");
        printf("1. Admin Login\n");
        printf("2. Book Classroom\n");
        printf("3. Vacate Classroom\n");
        printf("4. View Timetable\n");
        printf("5. View Classroom Details\n");
        printf("6. Exit\n");
        choice = read_int("Enter choice: ");

        if (choice == 1) {
            logged_in = admin_login();
        } 
        else if (choice == 2) {
            if (!logged_in) {
                printf("Please login as admin first.\n");
                continue;
            }
            int id = read_int("Enter Classroom ID: ");
            char faculty[NAME_LEN], subject[SUBJECT_LEN];
            read_str("Enter Faculty Name: ", faculty, sizeof(faculty));
            read_str("Enter Subject: ", subject, sizeof(subject));
            int st = read_int("Enter Start Time (e.g., 900 for 9:00): ");
            int et = read_int("Enter End Time (e.g., 1030 for 10:30): ");
            int res = book_classroom(id, faculty, subject, st, et);
            if (res == 1) printf("Booking Successful!\n");
            else if (res == -1) printf("Classroom not found.\n");
            else if (res == -2) printf("Invalid time range.\n");
            else if (res == -3) printf("Time slot conflict. Booking failed.\n");
        } 
        else if (choice == 3) {
            if (!logged_in) {
                printf("Please login as admin first.\n");
                continue;
            }
            int id = read_int("Enter Classroom ID: ");
            int st = read_int("Enter Start Time of slot to vacate: ");
            int et = read_int("Enter End Time of slot to vacate: ");
            int res = vacate_classroom(id, st, et);
            if (res == 1) printf("Classroom slot vacated successfully.\n");
            else if (res == -1) printf("Classroom not found.\n");
            else if (res == -2) printf("Slot not found.\n");
        } 
        else if (choice == 4) {
            display_timetable();
        } 
        else if (choice == 5) {
            int id = read_int("Enter Classroom ID: ");
            display_classroom(id);
        } 
        else if (choice == 6) {
            printf("Exiting... Goodbye!\n");
            free_all();
            break;
        } 
        else {
            printf("Invalid choice. Try again.\n");
        }
    }
    return 0;
}