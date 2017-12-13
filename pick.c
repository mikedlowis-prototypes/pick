#include <c.h>
#include <vec.h>
#include <termbox.h>
#include <ctype.h>

typedef struct {
    float score;
    char* string;
    size_t length;
    size_t match_start;
    size_t match_end;
} Choice;

char* ARGV0;
int AltFlag = 1; // Disable alternate screen usage
char Query[1024] = {0};
size_t QueryIdx = 0;
vec_t Choices = {0};
size_t ChoiceIdx = 2;

int by_score(const void* a, const void* b) {
    Choice* ca = ((Choice*)a);
    Choice* cb = ((Choice*)b);
    if (ca->score < cb->score)
        return 1;
    else if (ca->score > cb->score)
        return -1;
    else
        return strcmp(ca->string, cb->string);
}

void load_choices(void) {
    char* choice_text;
    Choice choice = {0};
    vec_init(&Choices, sizeof(Choice));
    while ((choice_text = rdline(stdin)) != NULL) {
        choice_text[strlen(choice_text)-1] = '\0';
        if (strlen(choice_text) > 0) {
            choice.string = choice_text;
            choice.length = strlen(choice_text);
            choice.score  = 1.0;
            vec_push_back(&Choices, &choice);
        }
    }
    vec_sort(&Choices, by_score);
}

char* find_match_start(char *str, int ch) {
    for (; *str; str++)
        if (tolower(*str) == tolower(ch))
            return str;
    return NULL;
}

bool match(char *string, size_t offset, size_t *start, size_t *end) {
    char* q = Query;
    char* s = find_match_start(&string[offset], *q);
    char* e = s;
    /* bail if no match for first char */
    if (s == NULL) return 0;
    /* find the end of the match */
    for (; *q; q++)
        if ((e = find_match_start(e, *q)) == NULL)
            return false;
    /* make note of the matching range */
    *start = s - string;
    *end   = e - string;
    /* Less than or equal is used in order to obtain the left-most match. */
    if (match(string, offset + 1, start, end) && (size_t)(e - s) <= *end - *start) {
        *start = s - string;
        *end   = e - string;
    }
    return true;
}

void score(void) {
    for (int i = 0; i < vec_size(&Choices); i++) {
        Choice* choice = (Choice*)vec_at(&Choices, i);
        float qlen = (float)QueryIdx;
        if (match(choice->string, 0, &choice->match_start, &choice->match_end)) {
            float clen = (float)(choice->match_end - choice->match_start);
            choice->score = qlen / clen / (float)(choice->length);
        } else {
            choice->match_start = 0;
            choice->match_end   = 0;
            choice->score       = 0.0f;
        }
    }
    vec_sort(&Choices, by_score);
}

void redraw(void) {
    tb_clear();
    /* Draw query and cursor */
    int max_x = (tb_width() < 1023 ? tb_width() : 1023);
    for (int x = 0; x < max_x; x++)
        tb_change_cell(x, 0, Query[x], TB_DEFAULT, TB_DEFAULT);
    tb_set_cursor(QueryIdx,0);
    /* Draw line dividing query and results */
    for (int x = 0; x < tb_width(); x++)
        tb_change_cell(x, 1, 0x2500, TB_DEFAULT, TB_DEFAULT);
    /* Draw the scored and sorted results */
    for (int i = 0, y = 2; (i < vec_size(&Choices)) && (y < tb_height()); i++) {
        bool selected = (y == ChoiceIdx);
        Choice* choice = (Choice*)vec_at(&Choices, i);
        if (choice->score >= 0.0) {
            for (int x = 0; choice->string[x] && x < tb_width(); x++) {
                bool inmatch  = (choice->match_end && x >= choice->match_start && x <= choice->match_end);
                tb_change_cell(x, i+2, choice->string[x],
                    (inmatch  ? TB_UNDERLINE|TB_BLUE : TB_DEFAULT),
                    (selected ? TB_WHITE : TB_DEFAULT));
            }
            for (int x = choice->length; selected && (x < tb_width()); x++)
                tb_change_cell(x, y, ' ', TB_DEFAULT, TB_WHITE);
            y++;
        }
    }
    tb_present();
}

void filter(void) {
    struct tb_event ev = {0};
    tb_init_with(AltFlag ? TB_INIT_EVERYTHING : 0);
    do {
        if (ev.type == TB_EVENT_KEY) {
            if (ev.key == TB_KEY_ENTER) {
                break;
            } else if (ev.key == TB_KEY_ESC) {
                ChoiceIdx = SIZE_MAX;
                break;
            } else if (ev.key == TB_KEY_BACKSPACE || ev.key == TB_KEY_BACKSPACE2) {
                if (QueryIdx > 0)
                    Query[--QueryIdx] = '\0';
            } else if (ev.key == TB_KEY_ARROW_DOWN) {
                if (ChoiceIdx < tb_width() && ChoiceIdx <= vec_size(&Choices))
                    ChoiceIdx++;
            } else if (ev.key == TB_KEY_ARROW_UP) {
                if (ChoiceIdx > 2)
                    ChoiceIdx--;
            } else if (ev.ch) {
                if (QueryIdx < sizeof(Query)-1)
                    Query[QueryIdx++] = ev.ch;
            }
        }
        score();
        redraw();
    } while (tb_poll_event(&ev));
    tb_shutdown();
}

int main(int argc, char** argv) {
    OPTBEGIN {
        case 'a': AltFlag = 0; break;
    } OPTEND;
    load_choices();
    if (vec_size(&Choices) > 1)
        filter();
    Choice* choice = (Choice*)vec_at(&Choices, ChoiceIdx-2);
    if (vec_size(&Choices) && ChoiceIdx != SIZE_MAX)
        printf("%s\n", choice->string);
    return 0;
}
