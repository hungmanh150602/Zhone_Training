#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_USERS 5
#define NAME_SIZE 32

typedef struct {
    int id;
    char name[NAME_SIZE];
    int score;
} User;

typedef struct {
    User *users;
    int count;
} Database;


/* Create database */
Database *database_create(int count)
{
    Database *db;

    db = malloc(sizeof(Database));

    if (db == NULL) {
        return NULL;
    }

    db->users = malloc(sizeof(User) * count);

    if (db->users == NULL) {
        free(db);
        return NULL;
    }

    db->count = count;

    return db;
}

/* Initialize users */
void initialize_users(Database *db)
{
    int i;

    for (i = 0; i < db->count; i++) {
        db->users[i].id = i + 1;

        snprintf(
            db->users[i].name,
            NAME_SIZE,
            "User_%d",
            i + 1
        );

        db->users[i].score = (i + 1) * 10;
    }
}

/* Print one user */
void print_user(const User *user)
{
    printf(
        "ID: %d | Name: %s | Score: %d\n",
        user->id,
        user->name,
        user->score
    );
}

/* Find user by ID */
User *find_user(Database *db, int id)
{
    int i;

    for (i = 0; i < db->count; i++) {
        if (db->users[i].id == id) {
            return &db->users[i];
        }
    }

    return NULL;
}

/* Increase user's score */
void increase_score(User *user, int amount)
{
    user->score += amount;
}

/* Process user */
void process_user(Database *db, int id)
{
    User *user;

    printf("\nSearching for user %d...\n", id);

    user = find_user(db, id);

    printf("User found. Updating score...\n");

    increase_score(user, 5);

    printf("Updated user:\n");
    print_user(user);
}

/* Print database */
void print_database(Database *db)
{
    int i;

    printf("\n========== DATABASE ==========\n");

    for (i = 0; i < db->count; i++) {
        print_user(&db->users[i]);
    }

    printf("==============================\n");
}

/* Cleanup */
void database_destroy(Database *db)
{
    if (db == NULL) {
        return;
    }

    free(db->users);
    free(db);
}

/* Main */
int main(void)
{
    Database *db;

    printf("Starting application...\n");

    db = database_create(MAX_USERS);

    if (db == NULL) {
        fprintf(stderr, "Failed to create database\n");
        return EXIT_FAILURE;
    }

    initialize_users(db);

    print_database(db);

    process_user(db, 999);

    printf("\nApplication finished successfully.\n");

    database_destroy(db);

    return EXIT_SUCCESS;
}
