#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Schedule {
    int id;
    int roomId;
    char faculty[50];
    char subject[50];
    int startTime;
    int endTime;
    int day;
    char status[20];
} Schedule;

// In-memory storage (simplified)
static Schedule schedules[100];
static int schedule_count = 5;

int main() {
    // Initialize sample data
    schedules[0].id = 1; schedules[0].roomId = 101; strcpy(schedules[0].faculty, "Dr. Mehta"); strcpy(schedules[0].subject, "DSA"); schedules[0].startTime = 900; schedules[0].endTime = 1000; schedules[0].day = 1; strcpy(schedules[0].status, "confirmed");
    schedules[1].id = 2; schedules[1].roomId = 101; strcpy(schedules[1].faculty, "Prof. Sharma"); strcpy(schedules[1].subject, "AI"); schedules[1].startTime = 1000; schedules[1].endTime = 1100; schedules[1].day = 1; strcpy(schedules[1].status, "confirmed");
    schedules[2].id = 3; schedules[2].roomId = 101; strcpy(schedules[2].faculty, "Prof. Ritu"); strcpy(schedules[2].subject, "CN"); schedules[2].startTime = 1100; schedules[2].endTime = 1200; schedules[2].day = 2; strcpy(schedules[2].status, "confirmed");
    schedules[3].id = 4; schedules[3].roomId = 102; strcpy(schedules[3].faculty, "Prof. Tanvi"); strcpy(schedules[3].subject, "OS"); schedules[3].startTime = 930; schedules[3].endTime = 1030; schedules[3].day = 3; strcpy(schedules[3].status, "confirmed");
    schedules[4].id = 5; schedules[4].roomId = 103; strcpy(schedules[4].faculty, "Prof. Amit"); strcpy(schedules[4].subject, "DBMS"); schedules[4].startTime = 900; schedules[4].endTime = 1000; schedules[4].day = 4; strcpy(schedules[4].status, "pending");

    char buffer[4096];
    size_t bytes_read = fread(buffer, 1, sizeof(buffer) - 1, stdin);
    buffer[bytes_read] = '\0';

    int booking_id = 0;
    char role[20] = "";
    char new_status[20] = "";

    sscanf(buffer, "{\"id\":%d,\"status\":\"%19[^\"]\",\"role\":\"%19[^\"]\"", 
           &booking_id, new_status, role);

    if (booking_id == 0 || strlen(role) == 0) {
        printf("Content-Type: application/json\r\n\r\n");
        printf("{\"success\":false,\"message\":\"Missing required fields\"}");
        return 0;
    }

    // Role-based access control
    if (strcmp(role, "admin") != 0) {
        printf("Content-Type: application/json\r\n\r\n");
        printf("{\"success\":false,\"message\":\"Only administrators can update booking status\"}");
        return 0;
    }

    // Find and update the booking
    int found = 0;
    printf("Content-Type: application/json\r\n\r\n");

    for (int i = 0; i < schedule_count; i++) {
        if (schedules[i].id == booking_id) {
            char old_status[20];
            strcpy(old_status, schedules[i].status);
            strcpy(schedules[i].status, new_status);
            
            printf("{\"success\":true,\"message\":\"Booking status updated from %s to %s\",\"data\":{\"id\":%d,\"roomId\":%d,\"faculty\":\"%s\",\"subject\":\"%s\",\"status\":\"%s\"}}",
                   old_status, new_status, schedules[i].id, schedules[i].roomId, 
                   schedules[i].faculty, schedules[i].subject, schedules[i].status);
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("{\"success\":false,\"message\":\"Booking not found\"}");
    }

    return 0;
}
