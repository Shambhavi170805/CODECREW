#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Schedule {
    char faculty[50];
    char subject[50];
    int startTime;
    int endTime;
    int day;
    int pending;
    struct Schedule *next;
} Schedule;

typedef struct Classroom {
    int roomID;
    Schedule *scheduleList;
    struct Classroom *next;
} Classroom;

static Classroom *building = NULL;

const char* dayName(int day) {
    switch (day) {
        case 1: return "Mon";
        case 2: return "Tue";
        case 3: return "Wed";
        case 4: return "Thu";
        case 5: return "Fri";
        default: return "Unknown";
    }
}

void trim_newline(char *s) {
    size_t len = strlen(s);
    if (len == 0) return;
    if (s[len - 1] == '\n') s[len - 1] = '\0';
}

void addClassroom(int id) {
    Classroom *newRoom = (Classroom *)malloc(sizeof(Classroom));
    newRoom->roomID = id;
    newRoom->scheduleList = NULL;
    newRoom->next = building;
    building = newRoom;
}

Classroom* findRoom(int id) {
    Classroom *tmp = building;
    while (tmp) {
        if (tmp->roomID == id) return tmp;
        tmp = tmp->next;
    }
    return NULL;
}

int addSchedule(int id, const char faculty[], const char subject[], int start, int end, int day, int pending) {
    Classroom *room = findRoom(id);
    if (!room) return -1;

    Schedule *tmp = room->scheduleList;
    while (tmp) {
        if (tmp->day == day && !(end <= tmp->startTime || start >= tmp->endTime)) {
            return 0;
        }
        tmp = tmp->next;
    }

    Schedule *node = (Schedule *)malloc(sizeof(Schedule));
    strncpy(node->faculty, faculty, sizeof(node->faculty) - 1);
    node->faculty[sizeof(node->faculty) - 1] = '\0';
    strncpy(node->subject, subject, sizeof(node->subject) - 1);
    node->subject[sizeof(node->subject) - 1] = '\0';
    node->startTime = start;
    node->endTime = end;
    node->day = day;
    node->pending = pending;
    node->next = room->scheduleList;
    room->scheduleList = node;
    return 1;
}

void displayToday(int today) {
    printf("\n------ TODAY'S SCHEDULE (%s) ------\n", dayName(today));
    Classroom *r = building;
    while (r) {
        printf("Room %d:\n", r->roomID);
        Schedule *s = r->scheduleList;
        int any = 0;
        while (s) {
            if (s->day == today) {
                printf("  Faculty: %-12s | Subject: %-10s | Time: %04d-%04d",
                       s->faculty, s->subject, s->startTime, s->endTime);
                if (s->pending)
                    printf("  [Pending Approval]");
                printf("\n");
                any = 1;
            }
            s = s->next;
        }
        if (!any) printf("  (No classes scheduled today)\n");
        r = r->next;
    }
}

int valid_hhmm(int t) {
    if (t < 0 || t > 2359) return 0;
    int hh = t / 100;
    int mm = t % 100;
    if (hh < 0 || hh > 23) return 0;
    if (mm < 0 || mm > 59) return 0;
    return 1;
}

int bookVacantClass(int today, int start, int end, const char faculty[], const char subject[]) {
    Classroom *r = building;
    while (r) {
        int occupied = 0;
        Schedule *s = r->scheduleList;
        while (s) {
            if (s->day == today && !(end <= s->startTime || start >= s->endTime)) {
                occupied = 1;
                break;
            }
            s = s->next;
        }
        if (!occupied) {
            addSchedule(r->roomID, faculty, subject, start, end, today, 1);
            return r->roomID;
        }
        r = r->next;
    }
    return 0;
}

void clear_stdin(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

int main(void) {
    addClassroom(101);
    addClassroom(102);
    addClassroom(103);

    addSchedule(101, "Dr.Mehta", "DSA", 900, 1000, 1, 0);
    addSchedule(101, "Prof.Sharma", "AI", 1000, 1100, 1, 0);
    addSchedule(101, "Prof.Ritu", "CN", 1100, 1200, 2, 0);
    addSchedule(102, "Prof.Tanvi", "OS", 930, 1030, 3, 0);
    addSchedule(103, "Prof.Amit", "DBMS", 900, 1000, 4, 0);

    printf("Sign in as a faculty\n");
    char id[50], pass[50];
    printf("Enter Faculty ID: ");
    fgets(id, sizeof(id), stdin);
    trim_newline(id);

    printf("Enter Password: ");
    fgets(pass, sizeof(pass), stdin);
    trim_newline(pass);

    if (strcmp(id, "test") != 0 || strcmp(pass, "1234") != 0) {
        printf("Invalid ID or password. Access denied.\n");
        return 0;
    }

    printf("\nLogin successful. Welcome, %s!\n", id);

    int today = 0;
    while (1) {
        printf("Enter today's day number (1=Mon ... 5=Fri): ");
        if (scanf("%d", &today) != 1) {
            printf("Invalid input. Try again.\n");
            clear_stdin();
            continue;
        }
        clear_stdin();
        if (today >= 1 && today <= 5) break;
        printf("Day must be between 1 and 5.\n");
    }

    displayToday(today);

    char faculty[50];
    char subject[50];
    int start = 0, end = 0;

    printf("\nEnter faculty name: ");
    fgets(faculty, sizeof(faculty), stdin);
    trim_newline(faculty);

    printf("Enter subject: ");
    fgets(subject, sizeof(subject), stdin);
    trim_newline(subject);

    while (1) {
        printf("Enter preferred START time (HHMM): ");
        if (scanf("%d", &start) != 1) {
            printf("Invalid input. Try again.\n");
            clear_stdin();
            continue;
        }
        clear_stdin();
        if (!valid_hhmm(start)) {
            printf("Invalid time. Try again.\n");
            continue;
        }
        break;
    }

    while (1) {
        printf("Enter preferred END time (HHMM): ");
        if (scanf("%d", &end) != 1) {
            printf("Invalid input. Try again.\n");
            clear_stdin();
            continue;
        }
        clear_stdin();
        if (!valid_hhmm(end) || end <= start) {
            printf("Invalid or smaller than start time. Try again.\n");
            continue;
        }
        break;
    }

    int roomSuggested = bookVacantClass(today, start, end, faculty, subject);
    if (roomSuggested > 0) {
        printf("\nRequested to book a class from %04d to %04d on %s for %s (%s).\n",
               start, end, dayName(today), subject, faculty);
        printf("Please wait for some time while the admin confirms the class number (suggested: %d).\n",
               roomSuggested);
    } else {
        printf("\nThere is no vacant class at that moment.\n");
    }

    displayToday(today);

    return 0;
}