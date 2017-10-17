#ifndef MOTION_H
#define MOTION_H

#include "command.h"

class GodotVim;

class Motion : public Command {
public:

    enum {
        INCLUSIVE = 1,
        LINEWISE = 1 << 1,
        TEXT_OBJECT = 1 << 2,
    };

    struct Pos {
        int row;
        int col;

        operator String() const {
            return "[" + itos(row) + ", " + itos(col) + "]";
        }

        bool operator < (const Pos &p_other) const {
            return (p_other.row == row) ? (col < p_other.col) : (row < p_other.row);
        }

        bool operator > (const Pos &p_other) const {
            return (p_other.row == row) ? (col > p_other.col) : (row > p_other.row);
        }

        bool operator == (const Pos &p_other) const {
            return (row == p_other.row) && (col == p_other.col);
        }

        Pos() {}
        Pos(int row, int col) : row(row), col(col) { }
    };

    struct Range {
        Pos from;
        Pos to;
        bool linewise;
        bool inclusive;
        bool text_object;

        Range() {}

        Range(int from_line, int from_col, int to_line, int to_col) :
            from(from_line, from_col),
            to(to_line, to_col) {}

        Range(Pos from, Pos to) : from(from), to(to) {}
    };

    typedef Pos (Motion::* motionFcn) ();
    typedef bool (* matchFunction) (int, String);

private:

    int flags;
    motionFcn fcn;

    Pos _move_backward(matchFunction function, bool accept_empty=false);
    Pos _move_forward(matchFunction function, bool accept_empty=false);
    Pos _move_to_first_non_blank(int line);
    Pos _move_by_columns(int cols);
    Pos _move_by_lines(int lines);

    Motion(GodotVim *vim, int flags, motionFcn fcn, int cmd_flags);

public:

    Pos _move_left();
    Pos _move_right();
    Pos _move_down();
    Pos _move_up();
    Pos _move_to_first_non_blank();
    Pos _move_to_line_end();
    Pos _move_to_line_start();
    Pos _move_word_right();
    Pos _move_word_right_big();
    Pos _move_word_beginning();
    Pos _move_word_beginning_big();
    Pos _move_word_end();
    Pos _move_word_end_big();
    Pos _move_word_end_backward();
    Pos _move_word_end_big_backward();
    Pos _move_paragraph_up();
    Pos _move_paragraph_down();
    Pos _move_to_matching_pair();
    Pos _move_to_beginning_of_last_line();
    Pos _move_to_beginning_of_first_line();
    Pos _move_to_beginning_of_previous_line();
    Pos _move_to_beginning_of_next_line();
    Pos _move_to_last_searched_char();
    Pos _move_to_last_searched_char_backward();
    Pos _move_to_column();

    Pos search_word_under_cursor();
    Pos search_word_under_cursor_backward();

    Pos find_next();
    Pos find_previous();

    Pos find_char();
    Pos find_char_backward();
    Pos find_till_char();
    Pos find_till_char_backward();


    bool is_inclusive();
    bool is_text_object();
    bool is_linewise();

    Pos apply();

    Range get_range();

    void run();

    Motion *get_motion() { return this; }

    bool is_motion() { return true; }

    static Motion *create_motion(GodotVim *vim, int flags, motionFcn fcn, int cmd_flags = 0);
};

#endif // MOTION_H
