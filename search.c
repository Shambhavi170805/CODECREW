#include <stdio.h>
#include <string.h>

#define MAX 5  // Number of classrooms

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
        {101, "Dr. Sharma", "Mathematics", 9, 10, 1},
        {102, "Prof. Verma", "Physics", 10, 11, 1},
        {103, "Dr. Singh", "Chemistry", 11, 12, 1},
        {104, "Ms. Nair", "English", 9, 11, 1},
        {105, "Mr. Rao", "Computer Science", 12, 13, 1}
    };

    int n = MAX;
    int i, choice, time;
    char fac[30];

    do {
        printf("\n=== Search & Display Menu ===\n");
        printf("1. Search vacant classrooms\n");
        printf("2. Display faculty locations\n");
        printf("3. Show dashboard\n");
        printf("0. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
        case 1:
            printf("Enter time (hour): ");
            scanf("%d", &time);
            printf("\nVacant classrooms at %d:00 are:\n", time);
            for (i = 0; i < n; i++) {
                if (time < cls[i].start_time || time >= cls[i].end_time)
                    printf("Room %d is VACANT\n", cls[i].room_no);
            }
            break;

        case 2:
            printf("Enter faculty name: ");
            scanf(" %[^\n]s", fac);
            {
                int found = 0;
                for (i = 0; i < n; i++) {
                    if (strcmp(fac, cls[i].faculty) == 0) {
                        printf("\n%s is in Room %d teaching '%s' from %d:00 to %d:00\n",
                               cls[i].faculty, cls[i].room_no,
                               cls[i].subject, cls[i].start_time, cls[i].end_time);
                        found = 1;
                    }
                }
                if (!found)
                    printf("Faculty not found or not teaching right now.\n");
            }
            break;

        case 3:
            printf("\n------ DASHBOARD ------\n");
            printf("Room\tStatus\t\tFaculty\t\t\tSubject\t\t\tTime\n");
            printf("-----------------------------------------------------------------------\n");
            for (i = 0; i < n; i++) {
                printf("%d\t", cls[i].room_no);
                printf("%s\t", cls[i].status ? "OCCUPIED" : "VACANT");
                printf("%-20s\t%-20s\t%d-%d\n",
                       cls[i].faculty, cls[i].subject,
                       cls[i].start_time, cls[i].end_time);
            }
            printf("-----------------------------------------------------------------------\n");
            break;

        case 0:
            printf("Exiting program.\n");
            break;

        default:
            printf("Invalid choice!\n");
        }

    } while (choice != 0);

    return 0;
}