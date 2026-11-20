#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TEXT_LEN   256
#define OPT_LEN    128
#define NAME_LEN   64
#define NUM_OPTS   4

typedef struct
{
    char text[TEXT_LEN];
    char options[NUM_OPTS][OPT_LEN];
    int  correct;             
    int  marks;
} Question;

typedef struct
{
    char name[NAME_LEN];
    int  score;
    int  totalMarks;
    int  timeTaken;            
    int  count;               
    int *order;               
    int *chosen;              
} Result;

Question *bank = NULL;        
int bankCount = 0;
int bankCap   = 0;

Result *results = NULL;        
int resCount = 0;
int resCap   = 0;

void readLine(const char *prompt, char *buf, int size)
{
    printf("%s", prompt);
    if (fgets(buf, size, stdin) == NULL)
    {
        buf[0] = '\0';
        return;
    }
    buf[strcspn(buf, "\n")] = '\0';
}

int readInt(const char *prompt, int min, int max)
{
    char line[64];
    int v;
    while (1)
    {
        printf("%s", prompt);
        if (fgets(line, sizeof(line), stdin) == NULL) return min;
        if (sscanf(line, "%d", &v) == 1 && v >= min && v <= max)
            return v;
        printf("  Invalid input. Enter a number between %d and %d.\n", min, max);
    }
}

void addToBank(Question q) 
{
    if (bankCount == bankCap)
    {
        int newCap = (bankCap == 0) ? 4 : bankCap * 2;
        Question *tmp = (Question *)realloc(bank, newCap * sizeof(Question));
        if (tmp == NULL)
        {
            printf("Memory allocation failed!\n");
            return;
        }
        bank = tmp;
        bankCap = newCap;
    }
    bank[bankCount++] = q;
}

void addQuestion(void) 
{
    Question q;
    char prompt[32];
    int i;

    printf("\n--- Add New Question ---\n");
    readLine("Question: ", q.text, TEXT_LEN);
    if (strlen(q.text) == 0)
    {
        printf("Question cannot be empty.\n");
        return;
    }
    for (i = 0; i < NUM_OPTS; i++)
     {
        sprintf(prompt, "Option %d: ", i + 1);
        readLine(prompt, q.options[i], OPT_LEN);
    }
    q.correct = readInt("Correct option (1-4): ", 1, NUM_OPTS);
    q.marks   = readInt("Marks for this question (1-100): ", 1, 100);

    addToBank(q);
    printf("Question added. Total questions: %d\n", bankCount);
}

void viewQuestions(void)
 {
    int i, j;
    if (bankCount == 0) 
    {
        printf("\nQuestion bank is empty.\n");
        return;
    }
    printf("\n--- Question Bank (%d questions) ---\n", bankCount);
    for (i = 0; i < bankCount; i++) 
    {
        Question *q = &bank[i];
        printf("\nQ%d. %s  [%d marks]\n", i + 1, q->text, q->marks);
        for (j = 0; j < NUM_OPTS; j++)
            printf("   %d) %s\n", j + 1, q->options[j]);
        printf("   Correct: %d\n", q->correct);
    }
}

void deleteQuestion(void) 
{
    int idx, i;
    if (bankCount == 0) 
    {
        printf("\nQuestion bank is empty.\n");
        return;
    }
    viewQuestions();
    idx = readInt("\nEnter question number to delete (0 to cancel): ", 0, bankCount);
    if (idx == 0) return;
    for (i = idx - 1; i < bankCount - 1; i++)
        bank[i] = bank[i + 1];
    bankCount--;
    printf("Question deleted.\n");
}

void loadSampleQuestions(void)
 {
    Question s[5] = 
    {
        {"Which keyword is used to define a structure in C?",
         {"struct", "union", "class", "typedef"}, 1, 2},
        {"Which function allocates memory dynamically in C?",
         {"alloc()", "malloc()", "new()", "create()"}, 2, 2},
        {"What is the size of int on most 32/64-bit systems?",
         {"1 byte", "2 bytes", "4 bytes", "8 bytes"}, 3, 2},
        {"Which header file is needed for printf()?",
         {"stdlib.h", "string.h", "math.h", "stdio.h"}, 4, 1},
        {"Which operator is used to get the address of a variable?",
         {"&", "*", "->", "%"}, 1, 1}
    };
    int i;
    for (i = 0; i < 5; i++) addToBank(s[i]);
    printf("5 sample questions loaded.\n");
}

void saveQuestions(void) 
{
    FILE *fp = fopen("questions.txt", "w");
    int i, j;
    if (!fp)
     {
        printf("Could not open questions.txt for writing.\n");
        return;
    }
    fprintf(fp, "%d\n", bankCount);
    for (i = 0; i < bankCount; i++)
     {
        fprintf(fp, "%s\n", bank[i].text);
        for (j = 0; j < NUM_OPTS; j++)
            fprintf(fp, "%s\n", bank[i].options[j]);
        fprintf(fp, "%d %d\n", bank[i].correct, bank[i].marks);
    }
    fclose(fp);
    printf("Saved %d questions to questions.txt\n", bankCount);
}

void loadQuestions(void) 
{
    FILE *fp = fopen("questions.txt", "r");
    int n, i, j;
    char line[TEXT_LEN + 8];
    if (!fp)
     {
        printf("questions.txt not found.\n");
        return;
    }
    if (fgets(line, sizeof(line), fp) == NULL || sscanf(line, "%d", &n) != 1) 
    {
        printf("File format error.\n");
        fclose(fp);
        return;
    }
    for (i = 0; i < n; i++) 
    {
        Question q;
        if (!fgets(q.text, TEXT_LEN, fp)) break;
        q.text[strcspn(q.text, "\n")] = '\0';
        for (j = 0; j < NUM_OPTS; j++) {
            if (!fgets(q.options[j], OPT_LEN, fp)) break;
            q.options[j][strcspn(q.options[j], "\n")] = '\0';
        }
        if (!fgets(line, sizeof(line), fp)) break;
        if (sscanf(line, "%d %d", &q.correct, &q.marks) != 2) break;
        addToBank(q);
    }
    fclose(fp);
    printf("Loaded questions. Total in bank: %d\n", bankCount);
}

void shuffle(int *arr, int n) 
{
    int i, j, t;
    for (i = n - 1; i > 0; i--)
     {
        j = rand() % (i + 1);
        t = arr[i]; arr[i] = arr[j]; arr[j] = t;
    }
}

Result *newResult(void) 
{
    if (resCount == resCap) 
    {
        int newCap = (resCap == 0) ? 4 : resCap * 2;
        Result *tmp = (Result *)realloc(results, newCap * sizeof(Result));
        if (tmp == NULL) return NULL;
        results = tmp;
        resCap = newCap;
    }
    return &results[resCount++];
}

void takeQuiz(void) 
{
    char name[NAME_LEN];
    int i, n, limitMin, limitSec, ans;
    int *order;
    Result *r;
    time_t start, now;

    if (bankCount == 0) 
    {
        printf("\nNo questions available. Ask the teacher to add some.\n");
        return;
    }

    readLine("\nEnter student name: ", name, NAME_LEN);
    if (strlen(name) == 0) strcpy(name, "Unknown");

    n = readInt("How many questions do you want to attempt? ", 1, bankCount);
    limitMin = readInt("Time limit in minutes (1-120): ", 1, 120);
    limitSec = limitMin * 60;

    order = (int *)malloc(bankCount * sizeof(int));
    if (!order) { printf("Memory error.\n"); return; }
    for (i = 0; i < bankCount; i++) order[i] = i;
    shuffle(order, bankCount);

    r = newResult();
    if (!r) { free(order); printf("Memory error.\n"); return; }
    strcpy(r->name, name);
    r->score = 0;
    r->totalMarks = 0;
    r->count = n;
    r->order  = (int *)malloc(n * sizeof(int));
    r->chosen = (int *)calloc(n, sizeof(int));
    if (!r->order || !r->chosen)
     {
        printf("Memory error.\n");
        free(order); free(r->order); free(r->chosen);
        resCount--;
        return;
    }

    printf("\n===== QUIZ STARTED =====\n");
    printf("Answer with 1-4. Enter 0 to skip.\n");
    start = time(NULL);

    for (i = 0; i < n; i++)
     {
        Question *q;
        int j, remaining;

        now = time(NULL);
        remaining = limitSec - (int)difftime(now, start);
        if (remaining <= 0) 
        {
            printf("\n*** Time is up! Remaining questions are marked unanswered. ***\n");
            for (; i < n; i++) {
                r->order[i] = order[i];
                r->chosen[i] = 0;
                r->totalMarks += bank[order[i]].marks;
            }
            break;
        }

        r->order[i] = order[i];
        q = &bank[order[i]];
        r->totalMarks += q->marks;

        printf("\nQ%d/%d (%d marks) [Time left: %d:%02d]\n",
               i + 1, n, q->marks, remaining / 60, remaining % 60);
        printf("%s\n", q->text);
        for (j = 0; j < NUM_OPTS; j++)
            printf("   %d) %s\n", j + 1, q->options[j]);

        ans = readInt("Your answer: ", 0, NUM_OPTS);
        r->chosen[i] = ans;
        if (ans == q->correct)
            r->score += q->marks;
    }

    now = time(NULL);
    r->timeTaken = (int)difftime(now, start);
    if (r->timeTaken > limitSec) r->timeTaken = limitSec;

    free(order);

    printf("\n===== QUIZ FINISHED =====\n");
    printf("%s, you scored %d / %d in %d min %d sec.\n",
           r->name, r->score, r->totalMarks,
           r->timeTaken / 60, r->timeTaken % 60);
    printf("Use 'Reports' from the main menu to see detailed feedback.\n");
}

void printStudentReport(FILE *out, Result *r) 
{
    int i, j;
    double pct = (r->totalMarks > 0) ? (r->score * 100.0 / r->totalMarks) : 0.0;

    fprintf(out, "\n======================================\n");
    fprintf(out, "Student   : %s\n", r->name);
    fprintf(out, "Score     : %d / %d (%.1f%%)\n", r->score, r->totalMarks, pct);
    fprintf(out, "Time taken: %d min %d sec\n", r->timeTaken / 60, r->timeTaken % 60);
    fprintf(out, "--------------------------------------\n");
    fprintf(out, "Per-question feedback:\n");

    for (i = 0; i < r->count; i++) 
    {
        Question *q = &bank[r->order[i]];
        fprintf(out, "\n Q%d. %s\n", i + 1, q->text);
        if (r->chosen[i] == 0) 
        {
            fprintf(out, "    Result : UNANSWERED (0/%d)\n", q->marks);
            fprintf(out, "    Correct: %d) %s\n", q->correct, q->options[q->correct - 1]);
        } else if (r->chosen[i] == q->correct) 
        {
            fprintf(out, "    Result : CORRECT (+%d)\n", q->marks);
            fprintf(out, "    Answer : %d) %s\n", r->chosen[i], q->options[r->chosen[i] - 1]);
        } else 
        {
            j = r->chosen[i];
            fprintf(out, "    Result : WRONG (0/%d)\n", q->marks);
            fprintf(out, "    Yours  : %d) %s\n", j, q->options[j - 1]);
            fprintf(out, "    Correct: %d) %s\n", q->correct, q->options[q->correct - 1]);
        }
    }
    fprintf(out, "======================================\n");
}

void classSummary(FILE *out)
{
    int i, top;
    double sum = 0.0, avg, topPct, pct;

    if (resCount == 0) 
    {
        fprintf(out, "\nNo results available.\n");
        return;
    }
    for (i = 0; i < resCount; i++)
        sum += (results[i].totalMarks > 0)
               ? results[i].score * 100.0 / results[i].totalMarks : 0.0;
    avg = sum / resCount;

    fprintf(out, "\n========== CLASS SUMMARY ==========\n");
    fprintf(out, "Students attempted : %d\n", resCount);
    fprintf(out, "Class average      : %.1f%%\n", avg);

    topPct = -1.0;
    for (i = 0; i < resCount; i++) 
    {
        pct = (results[i].totalMarks > 0)
              ? results[i].score * 100.0 / results[i].totalMarks : 0.0;
        if (pct > topPct) topPct = pct;
    }
    fprintf(out, "Top performer(s) (%.1f%%):\n", topPct);
    top = 0;
    for (i = 0; i < resCount; i++) 
    {
        pct = (results[i].totalMarks > 0)
              ? results[i].score * 100.0 / results[i].totalMarks : 0.0;
        if (pct == topPct) {
            fprintf(out, "   - %s (%d/%d, %d sec)\n", results[i].name,
                    results[i].score, results[i].totalMarks, results[i].timeTaken);
            top++;
        }
    }
    fprintf(out, "===================================\n");
}

void showLeaderboard(void) 
{
    int i, j;
    int *idx;
    if (resCount == 0) 
    {
        printf("\nNo results available.\n");
        return;
    }
    idx = (int *)malloc(resCount * sizeof(int));
    if (!idx) return;
    for (i = 0; i < resCount; i++) idx[i] = i;

    for (i = 0; i < resCount - 1; i++)
        for (j = 0; j < resCount - 1 - i; j++) 
    {
            Result *a = &results[idx[j]], *b = &results[idx[j + 1]];
            if (b->score > a->score ||
                (b->score == a->score && b->timeTaken < a->timeTaken)) {
                int t = idx[j]; idx[j] = idx[j + 1]; idx[j + 1] = t;
            }
        }

    printf("\n---------- LEADERBOARD ----------\n");
    printf("%-5s %-20s %-10s %-8s\n", "Rank", "Name", "Score", "Time(s)");
    for (i = 0; i < resCount; i++) {
        Result *r = &results[idx[i]];
        printf("%-5d %-20s %3d/%-6d %-8d\n", i + 1, r->name,
               r->score, r->totalMarks, r->timeTaken);
    }
    free(idx);
}

void reportsMenu(void) {
    int choice, i;
    if (resCount == 0) {
        printf("\nNo student has taken the quiz yet.\n");
        return;
    }
    do {
        printf("\n--- Reports ---\n");
        printf("1. Individual student report\n");
        printf("2. All student reports\n");
        printf("3. Class average & top performers\n");
        printf("4. Leaderboard\n");
        printf("0. Back\n");
        choice = readInt("Choice: ", 0, 4);

        if (choice == 1) {
            printf("\nStudents:\n");
            for (i = 0; i < resCount; i++)
                printf("  %d) %s\n", i + 1, results[i].name);
            i = readInt("Select student number: ", 1, resCount);
            printStudentReport(stdout, &results[i - 1]);
        } else if (choice == 2) {
            for (i = 0; i < resCount; i++)
                printStudentReport(stdout, &results[i]);
        } else if (choice == 3) {
            classSummary(stdout);
        } else if (choice == 4) {
            showLeaderboard();
        }
    } while (choice != 0);
}

void saveResults(void) {
    FILE *fp;
    int i;
    if (resCount == 0) {
        printf("\nNo results to save.\n");
        return;
    }
    fp = fopen("results.txt", "w");
    if (!fp) {
        printf("Could not open results.txt for writing.\n");
        return;
    }
    fprintf(fp, "QUIZ RESULTS REPORT\n");
    for (i = 0; i < resCount; i++)
        printStudentReport(fp, &results[i]);
    classSummary(fp);
    fclose(fp);
    printf("Results saved to results.txt\n");
}

/* ---------- Menus ---------- */
void teacherMenu(void) {
    int choice;
    do {
        printf("\n===== TEACHER MENU =====\n");
        printf("1. Add question\n");
        printf("2. View all questions\n");
        printf("3. Delete a question\n");
        printf("4. Load sample questions\n");
        printf("5. Save question bank to file\n");
        printf("6. Load question bank from file\n");
        printf("0. Back\n");
        choice = readInt("Choice: ", 0, 6);

        switch (choice) {
            case 1: addQuestion();         break;
            case 2: viewQuestions();       break;
            case 3: deleteQuestion();      break;
            case 4: loadSampleQuestions(); break;
            case 5: saveQuestions();       break;
            case 6: loadQuestions();       break;
        }
    } while (choice != 0);
}

void freeAll(void) {
    int i;
    for (i = 0; i < resCount; i++) {
        free(results[i].order);
        free(results[i].chosen);
    }
    free(results);
    free(bank);
}

int main(void) {
    int choice;
    srand((unsigned)time(NULL));

    printf("=======================================\n");
    printf("  DYNAMIC QUIZ CREATOR & EVALUATOR\n");
    printf("=======================================\n");

    do {
        printf("\n========== MAIN MENU ==========\n");
        printf("1. Teacher: manage questions\n");
        printf("2. Student: take quiz\n");
        printf("3. Reports (scores, feedback, class stats)\n");
        printf("4. Save results to file\n");
        printf("0. Exit\n");
        choice = readInt("Choice: ", 0, 4);

        switch (choice) {
            case 1: teacherMenu();  break;
            case 2: takeQuiz();     break;
            case 3: reportsMenu();  break;
            case 4: saveResults();  break;
            case 0: printf("\nGoodbye!\n"); break;
        }
    } while (choice != 0);

    freeAll();
    return 0;
}
