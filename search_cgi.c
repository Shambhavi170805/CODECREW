#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 5

struct Classroom {
    int room_no;
    char faculty[30];
    char subject[50];
    int start_time;   
    int end_time;     
    int status;       
};

int main() {
    struct Classroom cls[MAX] = {
        {101, "Dr. Mehta", "DSA", 9, 10, 1},
        {102, "Prof. Sharma", "AI", 10, 11, 1},
        {103, "Prof. Ritu", "CN", 11, 12, 1},
        {104, "Prof. Tanvi", "OS", 9, 11, 1},
        {105, "Prof. Amit", "DBMS", 12, 13, 1}
    };

    char buffer[4096];
    size_t bytes_read = fread(buffer, 1, sizeof(buffer) - 1, stdin);
    buffer[bytes_read] = '\0';

    char search_type[50] = "";
    sscanf(buffer, "{\"type\":\"%49[^\"]\"", search_type);

    printf("Content-Type: application/json\r\n\r\n");

    if (strcmp(search_type, "vacancy") == 0) {
        int time = 0;
        sscanf(buffer, "{\"type\":\"vacancy\",\"time\":%d", &time);
        printf("{\"success\":true,\"time\":%d,\"data\":[", time);
        int first = 1;
        for (int i = 0; i < MAX; i++) {
            if (time < cls[i].start_time || time >= cls[i].end_time) {
                if (!first) printf(",");
                printf("{\"id\":%d,\"capacity\":%d}", cls[i].room_no, 30 + i * 10);
                first = 0;
            }
        }
        printf("]}");
    } else if (strcmp(search_type, "classroom") == 0) {
        int room_id = 0;
        sscanf(buffer, "{\"type\":\"classroom\",\"id\":%d", &room_id);
        int found = 0;
        for (int i = 0; i < MAX; i++) {
            if (cls[i].room_no == room_id) {
                printf("{\"success\":true,\"data\":{\"id\":%d,\"faculty\":\"%s\",\"subject\":\"%s\",\"startTime\":%d,\"endTime\":%d}}", 
                       cls[i].room_no, cls[i].faculty, cls[i].subject, cls[i].start_time, cls[i].end_time);
                found = 1;
                break;
            }
        }
        if (!found) printf("{\"success\":false,\"message\":\"Classroom not found\"}");
    } else if (strcmp(search_type, "faculty") == 0) {
        char fac_name[50] = "";
        sscanf(buffer, "{\"type\":\"faculty\",\"name\":\"%49[^\"]\"", fac_name);
        printf("{\"success\":true,\"data\":[");
        int first = 1;
        for (int i = 0; i < MAX; i++) {
            if (strstr(cls[i].faculty, fac_name)) {
                if (!first) printf(",");
                printf("{\"faculty\":\"%s\",\"room\":%d,\"subject\":\"%s\"}", cls[i].faculty, cls[i].room_no, cls[i].subject);
                first = 0;
            }
        }
        printf("]}");
    } else {
        printf("{\"success\":false,\"message\":\"Unknown search type\"}");
    }

    return 0;
}
