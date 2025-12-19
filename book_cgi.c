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
    add_classroom(104);
    add_classroom(105);
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
    new_slot->next = room->slots;
    room->slots = new_slot;
    room->status = 1;
    return 1;
}

int main() {
    init_classrooms();

    char buffer[4096];
    size_t bytes_read = fread(buffer, 1, sizeof(buffer) - 1, stdin);
    buffer[bytes_read] = '\0';

    // Simple JSON parsing
    int room_id = 0, start_time = 0, end_time = 0;
    char faculty[NAME_LEN] = "";
    char subject[SUBJECT_LEN] = "";

    sscanf(buffer, "{\"roomId\":%d,\"faculty\":\"%49[^\"]\",\"subject\":\"%49[^\"]\",\"startTime\":%d,\"endTime\":%d", 
           &room_id, faculty, subject, &start_time, &end_time);

    if (room_id == 0 || start_time == 0 || strlen(faculty) == 0) {
        printf("Content-Type: application/json\r\n\r\n");
        printf("{\"success\":false,\"message\":\"Missing required fields\"}");
        return 0;
    }

    int result = book_classroom(room_id, faculty, subject, start_time, end_time);

    printf("Content-Type: application/json\r\n\r\n");
    
    if (result == 1) {
        printf("{\"success\":true,\"message\":\"Classroom booked successfully\",\"roomId\":%d}", room_id);
    } else if (result == -1) {
        printf("{\"success\":false,\"message\":\"Classroom not found\"}");
    } else if (result == -2) {
        printf("{\"success\":false,\"message\":\"Invalid time slot\"}");
    } else if (result == -3) {
        printf("{\"success\":false,\"message\":\"Time slot conflict\"}");
    }

    return 0;
}
