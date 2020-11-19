/* Program to evaluate candidate routines for Robotic Process Automation.

  Skeleton program written by Artem Polyvyanyy, artem.polyvyanyy@unimelb.edu.au,
  September 2020, with the intention that it be modified by students
  to add functionality, as required by the assignment specification.

  Student Authorship Declaration:

  (1) I certify that except for the code provided in the initial skeleton
  file, the  program contained in this submission is completely my own
  individual work, except where explicitly noted by further comments that
  provide details otherwise.  I understand that work that has been developed
  by another student, or by me in collaboration with other students, or by
  non-students as a result of request, solicitation, or payment, may not be
  submitted for assessment in this subject.  I understand that submitting for
  assessment work developed by or in collaboration with other students or
  non-students constitutes Academic Misconduct, and may be penalized by mark
  deductions, or by other penalties determined via the University of
  Melbourne Academic Honesty Policy, as described at
  https://academicintegrity.unimelb.edu.au.

  (2) I also certify that I have not provided a copy of this work in either
  softcopy or hardcopy or any other form to any other student, and nor will I
  do so until after the marks are released. I understand that providing my
  work to other students, regardless of my intention or any undertakings made
  to me by that other student, is also Academic Misconduct.

  (3) I further understand that providing a copy of the assignment
  specification to any form of code authoring or assignment tutoring service,
  or drawing the attention of others to such services and code that may have
  been made available via such a service, may be regarded as Student General
  Misconduct (interfering with the teaching activities of the University
  and/or inciting others to commit Academic Misconduct).  I understand that
  an allegation of Student General Misconduct may arise regardless of whether
  or not I personally make use of such solutions or sought benefit from such
  actions.

   Signed by: [Louis Pan 1173574]
   Dated:     [27/10/2020]

*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <string.h>

/* #define's -----------------------------------------------------------------*/

#define ASIZE 26
#define LINE_LENGTH 500
#define FALSE 0
#define TRUE 1
#define IGNORE 2

#define VALID_ACTION 0
#define INVALID_ACTION 1

#define GOT_ROUTINE 0
#define NO_LINE_FOUND 1
#define HASH_FOUND 2

/* type definitions ----------------------------------------------------------*/

// state (values of the 26 Boolean variables)
typedef unsigned char state_t[ASIZE];

// action
typedef struct action action_t;
struct action {
    char name;        // action name
    state_t precon;   // precondition
    state_t effect;   // effect
};

// step in a trace
typedef struct step step_t;
struct step {
    action_t *action; // pointer to an action performed at this step
    step_t   *next;   // pointer to the next step in this trace
};

// trace (implemented as a linked list)
typedef struct {
    step_t *head;     // pointer to the step in the head of the trace
    step_t *tail;     // pointer to the step in the tail of the trace
} trace_t;

/* function prototypes -------------------------------------------------------*/
trace_t* make_empty_trace(void);
trace_t* insert_at_tail(trace_t*, action_t*);
void free_trace(trace_t*);

/* my function prototypes ----------------------------------------------------*/

void clear_newlines(char[]);
void read_initial(state_t);
void read_actions(trace_t **);
action_t *find_in_actions(trace_t *, char);
void read_stage_0(trace_t *, trace_t **);
int is_valid(trace_t *, state_t, int *);

int num_actions(trace_t *);
void write_state(state_t);

int read_stage_1_routine(trace_t *, trace_t **);

// add your function prototypes here ...

/* where it all happens ------------------------------------------------------*/
int
main(int argc, char *argv[]) {
    state_t initial = { 0 };
    read_initial(initial);

    trace_t *actions = make_empty_trace();
    read_actions(&actions);

    trace_t *stage_0 = make_empty_trace();
    read_stage_0(actions, &stage_0);

    printf("==STAGE 0===============================\n");
    printf("Number of distinct actions: %d\n", num_actions(actions));
    printf("Length of the input trace: %d\n", num_actions(stage_0));

    int valid_till_num = 0;
    int valid = is_valid(stage_0, initial, &valid_till_num);
    if (valid == VALID_ACTION) {
        printf("Trace status: valid\n");
    } else {
        printf("Trace status: invalid\n");
    }
    printf("----------------------------------------\n");
    printf("  abcdefghijklmnopqrstuvwxyz\n");

    /* Copy the initial state for making changes */
    state_t copy;
    memcpy(copy, initial, ASIZE);
    printf("> ");
    write_state(copy);
    printf("\n");

    /* Go through stage_0 objective function, we know 
       valid_till_num are correct, just set effect and
       write to stdout */
    step_t *ptr = stage_0->head;
    while (ptr && valid_till_num--) {
        for (int i = 0; i < ASIZE; i++) {
            if (ptr->action->effect[i] != IGNORE) {
                copy[i] = ptr->action->effect[i];
            }
        }

        printf("%c ", ptr->action->name);
        write_state(copy);
        printf("\n");

        ptr = ptr->next;
    }

    int first = 1;
    int stage_1_result;
    int first_result = 1;
    trace_t *routine;
    while (
        (stage_1_result = read_stage_1_routine(actions, &routine)) ==
            GOT_ROUTINE) {
        if (first) {
            printf("==STAGE 1===============================\n");
            first = 0;
        }

        if (first_result == 0) {
            printf("----------------------------------------\n");
        } else {
            first_result = 0;
        }
        printf("Candidate routine: ");

        /* stage_1 for reading routine to */
        state_t stage_1;
        memset(stage_1, IGNORE, ASIZE);
        ptr = routine->head;
        while (ptr) {
            printf("%c", ptr->action->name);

            /* Only effect */
            for (int i = 0; i < ASIZE; i++) {
                if (ptr->action->effect[i] != IGNORE)
                    stage_1[i] = ptr->action->effect[i];
            }
            ptr = ptr->next;
        }
        printf("\n");
        
        int index_found = 0;
        ptr = stage_0->head;

        /* For each pointer in the objective function, again go till
           end to check consecutive subsequence for same effect, no
           need to check for preconditions */
        while (ptr) {
            /* state for checking subsequence */
            state_t check_state;
            memset(check_state, IGNORE, ASIZE);
            step_t *second_ptr = ptr;

            int found_same = 0;
            while (second_ptr) {
                for (int i = 0; i < ASIZE; i++) {
                    if (second_ptr->action->effect[i] != IGNORE)
                        check_state[i] = second_ptr->action->effect[i];
                }

                /* Same effect found */
                if (memcmp(stage_1, check_state, ASIZE) == 0) {
                    found_same = 1;
                    break;
                }

                second_ptr = second_ptr->next;
            }

            if (found_same) {
                printf("%5d: ", index_found);
                step_t *go_through = ptr;
                while (go_through != second_ptr->next) {
                    printf("%c", go_through->action->name);
                    go_through = go_through->next;
                    index_found++;
                }
                printf("\n");
                ptr = second_ptr->next;
                continue;
            }

            ptr = ptr->next;
            index_found++;
        }
        free_trace(routine);
    }

    first_result = 1;
    /* '#' was found while stage 1 reading */
    if (stage_1_result == HASH_FOUND) {
        int stage_2_result;
        first = 1;

        while (
            (stage_2_result = read_stage_1_routine(actions, &routine)) ==
                GOT_ROUTINE) {
            if (first) {
                printf("==STAGE 2===============================\n");
                first = 0;
            }

            /* stage 2 requires state_t derived from initial state, not IGNORE */
            int good = 1;
            state_t stage_2;
            memcpy(stage_2, initial, ASIZE);

            /* Check if routine read was valid */
            ptr = routine->head;
            while (ptr) {
                for (int i = 0; i < ASIZE; i++) {
                    if (ptr->action->precon[i] != IGNORE) {
                        if (stage_2[i] != ptr->action->precon[i]) {
                            good = 0;
                            break;
                        }
                    }
                }

                if (good == 0) break;
                for (int i = 0; i < ASIZE; i++) {
                    if (ptr->action->effect[i] != IGNORE)
                        stage_2[i] = ptr->action->effect[i];
                }
                ptr = ptr->next;
            }

            if (first_result == 0) {
                printf("----------------------------------------\n");
            } else {
                first_result = 0;
            }

            printf("Candidate routine: ");
            ptr = routine->head;
            while (ptr) {
                printf("%c", ptr->action->name);
                ptr = ptr->next;
            }
            printf("\n");

            /* Same dual loop */
            int index_found = 0;
            ptr = stage_0->head;
            while (ptr) {
                /* Derived from initial same as stage_2 state */
                state_t check_state;
                memcpy(check_state, initial, ASIZE);

                /* We need to check precondition for stage 2, then set effect */
                int found_same = 0;
                step_t *second_ptr = ptr;
                while (second_ptr) {

                    /* If preconditions was not met, we ignore this consecutive subsequence */
                    good = 1;
                    for (int i = 0; i < ASIZE; i++) {
                        if (second_ptr->action->precon[i] != IGNORE) {
                            if (check_state[i] != second_ptr->action->precon[i]) {
                                good = 0;
                                break;
                            }
                        }
                    }

                    if (good == 0) break;
                    for (int i = 0; i < ASIZE; i++) {
                        if (second_ptr->action->effect[i] != IGNORE)
                            check_state[i] = second_ptr->action->effect[i];
                    }

                    /* Same effect found */
                    if (memcmp(stage_2, check_state, ASIZE) == 0) {
                        found_same = 1;
                        break;
                    }

                    second_ptr = second_ptr->next;
                }

                if (found_same) {
                    printf("%5d: ", index_found);
                    step_t *go_through = ptr;
                    while (go_through != second_ptr->next) {
                        printf("%c", go_through->action->name);
                        go_through = go_through->next;
                        index_found++;
                    }
                    printf("\n");
                    ptr = second_ptr->next;
                    continue;
                }

                ptr = ptr->next;
                index_found++;
            }

            free_trace(routine);
        }

        if (first == 0) {
            printf("==THE END===============================\n");
        }
    }
    
    ptr = actions->head;
    while (ptr) {
        free(ptr->action);
        ptr = ptr->next;
    }
    
    free_trace(actions);
    free_trace(stage_0);
    return EXIT_SUCCESS;        // we are done !!! algorithms are fun!!!
}

/* function definitions ------------------------------------------------------*/

// Adapted version of the make_empty_list function by Alistair Moffat:
// https://people.eng.unimelb.edu.au/ammoffat/ppsaa/c/listops.c
// Data type and variable names changed
trace_t
*make_empty_trace(void) {
    trace_t *R;
    R = (trace_t*)malloc(sizeof(*R));
    assert(R!=NULL);
    R->head = R->tail = NULL;
    return R;
}

// Adapted version of the insert_at_foot function by Alistair Moffat:
// https://people.eng.unimelb.edu.au/ammoffat/ppsaa/c/listops.c
// Data type and variable names changed
trace_t 
*insert_at_tail(trace_t* R, action_t* addr) {
    step_t *new;
    new = (step_t*)malloc(sizeof(*new));
    assert(R!=NULL && new!=NULL);
    new->action = addr;
    new->next = NULL;
    if (R->tail==NULL) { /* this is the first insertion into the trace */
        R->head = R->tail = new; 
    } else {
        R->tail->next = new;
        R->tail = new;
    }
    return R;
}

// Adapted version of the free_list function by Alistair Moffat:
// https://people.eng.unimelb.edu.au/ammoffat/ppsaa/c/listops.c
// Data type and variable names changed
void
free_trace(trace_t* R) {
    step_t *curr, *prev;
    assert(R!=NULL);
    curr = R->head;
    while (curr) {
        prev = curr;
        curr = curr->next;
        free(prev);
    }
    free(R);
}

/* my function definitions ---------------------------------------------------*/

// add your function definitions here ...

/*
   Remove \n or \r\n at the end of line
   Line was read by fgets
 */
void
clear_newlines(char line[]) {
    int len = strlen(line);
    if (len > 0) {
        if (line[len - 1] == '\n') {
            line[len - 1] = '\0';

            if (len - 1 > 0) {
                if (line[len - 2] == '\r') {
                    line[len - 2] = '\0';
                }
            }
        }
    }
}

/*
   Read initial state from stdin
   The function will keep reading and adding to initial until
   '#' is found
 */
void
read_initial(state_t initial) {
    while (1) {
        char line[LINE_LENGTH] = { 0 };
        void *result = fgets(line, LINE_LENGTH - 1, stdin);
        assert(result != NULL);

        clear_newlines(line);

        /* Constraint on line, only one character line is readable */
        assert(strlen(line) == 1);

        if (line[0] == '#') break;
        for (int i = 0; i < strlen(line); i++) {
            initial[line[i] - 'a'] = 1;
        }
    }
}

/*
   Read actions from stdin and adding to linkedlist of trace
   It will keep reading until '#' is found
 */
void
read_actions(trace_t **actions) {
    while (1) {
        char line[LINE_LENGTH] = { 0 };
        void *result = fgets(line, LINE_LENGTH - 1, stdin);
        assert(result != NULL);

        clear_newlines(line);
        assert(strlen(line) != 0);
        if (line[0] == '#') break;

        /* malloc'ed to add to trace */
        action_t *action = (action_t *)malloc(sizeof(action_t));
        assert(action != NULL);

        memset(action, 0, sizeof(action_t));
        memset(action->precon, IGNORE, ASIZE);
        memset(action->effect, IGNORE, ASIZE);

        int i = 0;
        /* 
           The following five loop are same,
           Just keep reading until ':' or '\0' (for last)
           is read, Fist preconditions are read for trueness
           then preconditions are read for falseness
           then action name and then effect in same order of preconditions
         */
        while (line[i] != ':') {
            action->precon[line[i] - 'a'] = TRUE;
            i++;
        }
        i++;

        while (line[i] != ':') {
            action->precon[line[i] - 'a'] = FALSE;
            i++;
        }
        i++;

        while (line[i] != ':') {
            action->name = line[i];
            i++;
        }
        i++;

        while (line[i] != ':') {
            action->effect[line[i] - 'a'] = TRUE;
            i++;
        }
        i++;

        while (line[i] != '\0') {
            action->effect[line[i] - 'a'] = FALSE;
            i++;
        }
        i++;

        /* Just add to trace_t at end */
        (*actions) = insert_at_tail((*actions), action);
    }
}

/*
   From the actions read from 'read_actions'
   Read the objective function, this output trace has
   same pointer to action as that to 'actions', name is
   searched in it.
 */
void
read_stage_0(trace_t *actions, trace_t **output) {
    char line[LINE_LENGTH];
    void *result = fgets(line, LINE_LENGTH - 1, stdin);
    assert(result != NULL);

    clear_newlines(line);
    for (int i = 0; i < strlen(line); i++) {
        (*output) = insert_at_tail((*output), find_in_actions(actions, line[i]));
    }

    fgets(line, LINE_LENGTH - 1, stdin);
}

/*
   From trace, find a action with name 'n'
   Return pointer to it or NULL
 */
action_t 
*find_in_actions(trace_t *trace, char n) {
    step_t *head = trace->head;
    while (head) {
        if (head->action->name == n) {
            return head->action;
        }

        head = head->next;
    }

    return NULL;
}

/*
   Using objective function and initial state
   Check if the function is valid or not
   If the function is invalid, 'valid_till_num'
   returns number of sub-actions that was correct in
   the whole function
 */
int 
is_valid(trace_t *actions, state_t initial, int *valid_till_num)
{
    state_t copy;
    memcpy(copy, initial, ASIZE);

    *valid_till_num = 0;
    step_t *ptr = actions->head;
    while (ptr) {
        /*
           Check preconditions, for validity
         */
        for (int i = 0; i < ASIZE; i++) {
            if (ptr->action->precon[i] != IGNORE) {
                if (copy[i] != ptr->action->precon[i]) {
                    return INVALID_ACTION;
                }
            }
        }

        /*
           If the preconditions were met, set effects
         */
        for (int i = 0; i < ASIZE; i++) {
            if (ptr->action->effect[i] != IGNORE) {
                copy[i] = ptr->action->effect[i];
            }
        }

        *valid_till_num = *valid_till_num + 1;
        ptr = ptr->next;
    }

    return VALID_ACTION;
}

/*
   Return number of actions in trace
 */
int
num_actions(trace_t *trace) {
    int num = 0;
    step_t *ptr = trace->head;
    while (ptr) {
        ++num;
        ptr = ptr->next;
    }

    return num;
}

/*
   Write state as ASIZE number of 0 or 1
 */
void
write_state(state_t state) {
    for (int i = 0; i < ASIZE; i++) {
        printf("%d", state[i]);
    }
}

/*
   Read a line containing a routine, also used for stage 2
   Returns NO_LINE_FOUND if error while reading line
           HASH_FOUND if '#' is found, for stage 1, it
                      indicates stage 2 is there
           GOT_ROUTINE if the routine was successfully read
 */
int
read_stage_1_routine(trace_t *actions, trace_t **routine) {
    (*routine) = make_empty_trace();

    char line[LINE_LENGTH] = { 0 };
    void *result = fgets(line, LINE_LENGTH - 1, stdin);
    if (result == NULL) {
        free_trace((*routine));
        return NO_LINE_FOUND;
    }

    clear_newlines(line);
    assert(strlen(line) != 0);
    if (line[0] == '#') {
        free_trace((*routine));
        return HASH_FOUND;
    }

    for (int i = 0; i < strlen(line); i++) {
        (*routine) = insert_at_tail((*routine), find_in_actions(actions, line[i]));
    }
    return GOT_ROUTINE;
}
// algorithms are fun 
/* ta-da-da-daa!!! -----------------------------------------------------------*/
