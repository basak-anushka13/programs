/*
 * Subject-wise Library Management System
 *
 * Demo console application for Physics, Chemistry, and Mathematics books.
 * Students can sign in only to their assigned subject; the administrator can
 * browse and manage books in every subject.
 *
 * Build: cc -std=c11 -Wall -Wextra -pedantic library_management.c -o library
 * Run:   ./library
 *
 * Demo credentials (change these before using in a real environment):
 *   admin / admin123
 *   physics1 / physics123
 *   chemistry1 / chemistry123
 *   maths1 / maths123
 *
 * This example stores books in memory and uses demonstration credentials.
 * It is intended for learning, not production authentication or storage.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_BOOKS 100
#define TITLE_LEN 100
#define AUTHOR_LEN 80
#define USER_LEN 32
#define PASS_LEN 32
#define INPUT_LEN 128

typedef enum {
    PHYSICS = 0,
    CHEMISTRY,
    MATHEMATICS,
    SUBJECT_COUNT,
    ADMIN = -1
} Role;

typedef struct {
    int id;
    char title[TITLE_LEN];
    char author[AUTHOR_LEN];
    Role subject;
    int available;
} Book;

typedef struct {
    const char *username;
    const char *password;
    Role role;
} User;

static Book books[MAX_BOOKS] = {
    {101, "Fundamentals of Physics", "David Halliday", PHYSICS, 1},
    {102, "Concepts of Modern Physics", "Arthur Beiser", PHYSICS, 1},
    {201, "Organic Chemistry", "Paula Yurkanis Bruice", CHEMISTRY, 1},
    {202, "Physical Chemistry", "Peter Atkins", CHEMISTRY, 1},
    {301, "Introduction to Calculus", "Gilbert Strang", MATHEMATICS, 1},
    {302, "Linear Algebra", "Howard Anton", MATHEMATICS, 1}
};
static size_t book_count = sizeof books / sizeof books[0];

static const char *subject_name(Role role) {
    switch (role) {
        case PHYSICS: return "Physics";
        case CHEMISTRY: return "Chemistry";
        case MATHEMATICS: return "Mathematics";
        case ADMIN: return "All subjects (Administrator)";
        default: return "Unknown";
    }
}

static int read_line(const char *prompt, char *buffer, size_t size) {
    int ch;
    printf("%s", prompt);
    if (!fgets(buffer, (int)size, stdin)) return 0;
    size_t len = strlen(buffer);
    if (len && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    } else {
        while ((ch = getchar()) != '\n' && ch != EOF) { }
    }
    return 1;
}

static int read_int(const char *prompt, int *value) {
    char line[INPUT_LEN], *end;
    long parsed;
    if (!read_line(prompt, line, sizeof line)) return 0;
    parsed = strtol(line, &end, 10);
    while (isspace((unsigned char)*end)) end++;
    if (line == end || *end != '\0' || parsed < -2147483647L - 1 || parsed > 2147483647L) {
        puts("Please enter a valid number.");
        return -1;
    }
    *value = (int)parsed;
    return 1;
}

static Role subject_from_choice(int choice) {
    if (choice == 1) return PHYSICS;
    if (choice == 2) return CHEMISTRY;
    return MATHEMATICS;
}

static Role login(void) {
    static const User users[] = {
        {"admin", "admin123", ADMIN},
        {"physics1", "physics123", PHYSICS},
        {"chemistry1", "chemistry123", CHEMISTRY},
        {"maths1", "maths123", MATHEMATICS}
    };
    char username[USER_LEN], password[PASS_LEN];
    if (!read_line("Username: ", username, sizeof username) ||
        !read_line("Password: ", password, sizeof password)) return (Role)99;

    for (size_t i = 0; i < sizeof users / sizeof users[0]; i++) {
        if (strcmp(username, users[i].username) == 0 &&
            strcmp(password, users[i].password) == 0) return users[i].role;
    }
    return (Role)99;
}

static int can_access(Role user, Role book_subject) {
    return user == ADMIN || user == book_subject;
}

static void list_books(Role user) {
    int found = 0;
    printf("\nBooks available to %s:\n", subject_name(user));
    printf("%-5s %-32s %-27s %-12s %s\n", "ID", "Title", "Author", "Subject", "Status");
    puts("-------------------------------------------------------------------------------");
    for (size_t i = 0; i < book_count; i++) {
        if (!can_access(user, books[i].subject)) continue;
        printf("%-5d %-32.32s %-27.27s %-12s %s\n", books[i].id,
               books[i].title, books[i].author, subject_name(books[i].subject),
               books[i].available ? "Available" : "Borrowed");
        found = 1;
    }
    if (!found) puts("No books found for this subject.");
}

static void borrow_book(Role user) {
    int id;
    int result = read_int("Enter book ID to borrow: ", &id);
    if (result <= 0) return;
    for (size_t i = 0; i < book_count; i++) {
        if (books[i].id != id) continue;
        if (!can_access(user, books[i].subject)) {
            puts("Access denied: you may borrow books only from your subject.");
        } else if (!books[i].available) {
            puts("That book is already borrowed.");
        } else {
            books[i].available = 0;
            printf("You borrowed '%s'.\n", books[i].title);
        }
        return;
    }
    puts("Book ID not found.");
}

static void return_book(Role user) {
    int id;
    int result = read_int("Enter book ID to return: ", &id);
    if (result <= 0) return;
    for (size_t i = 0; i < book_count; i++) {
        if (books[i].id != id) continue;
        if (!can_access(user, books[i].subject)) {
            puts("Access denied: you may return books only from your subject.");
        } else if (books[i].available) {
            puts("That book is already in the library.");
        } else {
            books[i].available = 1;
            printf("You returned '%s'.\n", books[i].title);
        }
        return;
    }
    puts("Book ID not found.");
}

static void add_book(void) {
    int choice, id;
    char title[TITLE_LEN], author[AUTHOR_LEN];
    if (book_count >= MAX_BOOKS) {
        puts("The library is full; cannot add more books.");
        return;
    }
    if (read_int("New book ID: ", &id) <= 0 || id <= 0) {
        puts("Book ID must be a positive number.");
        return;
    }
    for (size_t i = 0; i < book_count; i++) {
        if (books[i].id == id) {
            puts("That book ID already exists.");
            return;
        }
    }
    if (!read_line("Book title: ", title, sizeof title) || title[0] == '\0' ||
        !read_line("Author: ", author, sizeof author) || author[0] == '\0') {
        puts("Title and author cannot be empty.");
        return;
    }
    puts("Subject: 1) Physics  2) Chemistry  3) Mathematics");
    if (read_int("Choose subject: ", &choice) <= 0 || choice < 1 || choice > 3) {
        puts("Invalid subject.");
        return;
    }
    books[book_count].id = id;
    snprintf(books[book_count].title, sizeof books[book_count].title, "%s", title);
    snprintf(books[book_count].author, sizeof books[book_count].author, "%s", author);
    books[book_count].subject = subject_from_choice(choice);
    books[book_count].available = 1;
    book_count++;
    puts("Book added successfully.");
}

int main(void) {
    puts("=====================================");
    puts(" Subject-wise Library Management");
    puts("=====================================\n");
    puts("Demo accounts: admin/admin123, physics1/physics123,");
    puts("               chemistry1/chemistry123, maths1/maths123\n");

    for (;;) {
        Role user;
        puts("1) Log in   0) Exit");
        int choice;
        int result = read_int("Select: ", &choice);
        if (result == 0 || (result > 0 && choice == 0)) break;
        if (result < 0 || choice != 1) {
            puts("Please choose 1 or 0.\n");
            continue;
        }
        user = login();
        if ((int)user == 99) {
            puts("Invalid credentials.\n");
            continue;
        }
        printf("\nWelcome! Access: %s\n", subject_name(user));

        for (;;) {
            int action;
            puts("\n1) View books   2) Borrow book   3) Return book");
            if (user == ADMIN) puts("4) Add book");
            puts("0) Log out");
            result = read_int("Select: ", &action);
            if (result == 0 || (result > 0 && action == 0)) break;
            if (result < 0) continue;
            switch (action) {
                case 1: list_books(user); break;
                case 2: borrow_book(user); break;
                case 3: return_book(user); break;
                case 4:
                    if (user == ADMIN) add_book();
                    else puts("Invalid choice.");
                    break;
                default: puts("Invalid choice."); break;
            }
        }
        puts("\nLogged out.\n");
    }
    puts("Goodbye!");
    return 0;
}
