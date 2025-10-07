#include <stdio.h>
#include <string.h>

#define MAX 20

struct Class {
    char teacher[50];
    char subject[50];
    float start;
    float end;
    int day;   // 1=Mon,...,5=Fri
    int room;
};

struct Class schedule[MAX];
int count = 0;
int today;

void addClass(int room, int day, char teacher[], char subject[], float start, float end) {
    // check conflict
    for (int i = 0; i < count; i++) {
        if (schedule[i].room == room && schedule[i].day == day &&
            !(end <= schedule[i].start || start >= schedule[i].end)) {
            printf("Conflict: Room %d already booked %.1f–%.1f by %s (%s)\n",
                   room, schedule[i].start, schedule[i].end,
                   schedule[i].teacher, schedule[i].subject);
            return;
        }
    }

    struct Class c;
    c.room = room;
    c.day = day;
    c.start = start;
    c.end = end;
    strcpy(c.teacher, teacher);
    strcpy(c.subject, subject);
    schedule[count++] = c;

    printf("Added: Room %d %.1f–%.1f %s (%s)\n", room, start, end, teacher, subject);
}

void showDay(int day) {
    printf("\nSchedule for Day %d:\n", day);
    for (int room = 1; room <= 3; room++) {
        printf("Room %d:\n", room);
        int found = 0;
        for (float t = 8; t < 17; t++) {
            int booked = 0;
            for (int i = 0; i < count; i++) {
                if (schedule[i].room == room && schedule[i].day == day &&
                    t >= schedule[i].start && t < schedule[i].end) {
                    printf("  %.1f–%.1f: %s (%s)\n", schedule[i].start,
                           schedule[i].end, schedule[i].teacher, schedule[i].subject);
                    booked = 1;
                    break;
                }
            }
            if (!booked) printf("  %.1f–%.1f: Vacant\n", t, t + 1);
            found = 1;
        }
        if (!found) printf("  No entries.\n");
    }
}

void teacherView(char name[]) {
    printf("\nToday's schedule for %s (Day %d):\n", name, today);
    int found = 0;
    for (int i = 0; i < count; i++) {
        if (schedule[i].day == today && strcmp(schedule[i].teacher, name) == 0) {
            printf("Room %d %.1f–%.1f: %s\n", schedule[i].room,
                   schedule[i].start, schedule[i].end, schedule[i].subject);
            found = 1;
        }
    }
    if (!found) printf("No classes today.\n");
}

void teacherBook(char name[]) {
    int room;
    float start, end;
    char subject[50];
    printf("Enter room, subject, start, end: ");
    scanf("%d %s %f %f", &room, subject, &start, &end);
    addClass(room, today, name, subject, start, end);
}

void teacherVacate(char name[]) {
    int room;
    float start;
    printf("Enter room and start time to vacate: ");
    scanf("%d %f", &room, &start);
    for (int i = 0; i < count; i++) {
        if (schedule[i].room == room && schedule[i].day == today &&
            schedule[i].start == start && strcmp(schedule[i].teacher, name) == 0) {
            for (int j = i; j < count - 1; j++) schedule[j] = schedule[j + 1];
            count--;
            printf("Class removed.\n");
            return;
        }
    }
    printf("No such class found.\n");
}

int main() {
    printf("Enter today's day (1=Mon–5=Fri): ");
    scanf("%d", &today);

    printf("\nAdmin: Set timetable for week (example entries)\n");
    addClass(1, 1, "Alice", "Math", 8, 10);
    addClass(1, 1, "Bob", "Science", 10, 12);
    addClass(2, 2, "Charlie", "English", 9, 11);

    printf("\nFull Week Timetable:\n");
    for (int d = 1; d <= 5; d++) showDay(d);

    char name[50];
    printf("\nEnter teacher name to log in: ");
    scanf("%s", name);

    int opt;
    do {
        printf("\n1. View today's schedule\n2. Book class\n3. Vacate class\n4. View today's timetable\n5. Exit\nChoice: ");
        scanf("%d", &opt);
        switch (opt) {
            case 1: teacherView(name); break;
            case 2: teacherBook(name); break;
            case 3: teacherVacate(name); break;
            case 4: showDay(today); break;
        }
    } while (opt != 5);

    return 0;
}
