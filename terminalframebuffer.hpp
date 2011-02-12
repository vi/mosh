#ifndef TERMINALFB_HPP
#define TERMINALFB_HPP

#include <vector>
#include <deque>
#include <string>
#include <list>
#include <assert.h>

/* Terminal framebuffer */

namespace Terminal {
  class Renditions {
  public:
    bool bold, underlined, blink, inverse, invisible;
    int foreground_color;
    int background_color;

    Renditions( int s_background );
    void set_rendition( int num );
    std::string sgr( void );

    bool operator==( const Renditions &x )
    {
      return (bold == x.bold) && (underlined == x.underlined)
	&& (blink == x.blink) && (inverse == x.inverse)
	&& (invisible == x.invisible) && (foreground_color == x.foreground_color)
	&& (background_color == x.background_color);
    }
  };

  class Cell {
  public:
    std::vector<wchar_t> contents;
    char fallback; /* first character is combining character */
    int width;
    Renditions renditions;

    Cell( int background_color )
      : contents(),
	fallback( false ),
	width( 1 ),
	renditions( background_color )
    {}

    void reset( int background_color );

    inline bool operator==( const Cell &x )
    {
      return ( (contents == x.contents)
	       && (fallback == x.fallback)
	       && (width == x.width)
	       && (renditions == x.renditions) );
    }
  };

  class Row {
  public:
    std::vector<Cell> cells;
    bool wrap;

    Row( size_t s_width, int background_color )
      : cells( s_width, Cell( background_color ) ), wrap( false )
    {}

    void insert_cell( int col, int background_color );
    void delete_cell( int col, int background_color );

    void reset( int background_color );
  };

  class SavedCursor {
  public:
    int cursor_col, cursor_row;
    Renditions renditions;
    /* not implemented: character set shift state */
    bool auto_wrap_mode;
    bool origin_mode;
    /* not implemented: state of selective erase */

    SavedCursor();
  };

  class DrawState {
  private:
    int width, height;

    void new_grapheme( void );
    void snap_cursor_to_border( void );

    int cursor_col, cursor_row;
    int combining_char_col, combining_char_row;

    std::vector<bool> tabs;

    int scrolling_region_top_row, scrolling_region_bottom_row;

    Renditions renditions;

    SavedCursor save;

  public:
    bool next_print_will_wrap;
    bool origin_mode;
    bool auto_wrap_mode;
    bool insert_mode;
    bool cursor_visible;
    bool reverse_video;

    bool application_mode_cursor_keys;

    /* bold, etc. */

    void move_row( int N, bool relative = false );
    void move_col( int N, bool relative = false, bool implicit = false );

    int get_cursor_col( void ) { return cursor_col; }
    int get_cursor_row( void ) { return cursor_row; }
    int get_combining_char_col( void ) { return combining_char_col; }
    int get_combining_char_row( void ) { return combining_char_row; }
    int get_width( void ) { return width; }
    int get_height( void ) { return height; }

    void set_tab( void );
    void clear_tab( int col );
    int get_next_tab( void );

    std::vector<int> get_tabs( void );

    void set_scrolling_region( int top, int bottom );

    int get_scrolling_region_top_row( void ) { return scrolling_region_top_row; }
    int get_scrolling_region_bottom_row( void ) { return scrolling_region_bottom_row; }

    int limit_top( void );
    int limit_bottom( void );

    void add_rendition( int x ) { renditions.set_rendition( x ); }
    Renditions get_renditions( void ) { return renditions; }
    int get_background_rendition( void ) { return renditions.background_color; }

    void save_cursor( void );
    void restore_cursor( void );
    void clear_saved_cursor( void ) { save = SavedCursor(); }

    void resize( int s_width, int s_height );

    DrawState( int s_width, int s_height );
  };

  class Framebuffer {
  private:
    std::deque<Row> rows;
    std::vector<wchar_t> window_title;

    void scroll( int N );

    Row newrow( void ) { return Row( ds.get_width(), ds.get_background_rendition() ); }

  public:
    Framebuffer( int s_width, int s_height );
    DrawState ds;

    void move_rows_autoscroll( int rows );

    Row *get_row( int row )
    {
      if ( row == -1 ) row = ds.get_cursor_row();

      return &rows[ row ];
    }

    inline Cell *get_cell( void )
    {
#ifdef DEBUG
      assert( ds.get_cursor_row() >= 0 );
      assert( ds.get_cursor_col() >= 0 );
      assert( ds.get_cursor_row() < ds.get_height() );
      assert( ds.get_cursor_col() < ds.get_width() );
#endif

      return &rows[ ds.get_cursor_row() ].cells[ ds.get_cursor_col() ];
    }

    inline Cell *get_cell( int row, int col )
    {
      if ( row == -1 ) row = ds.get_cursor_row();
      if ( col == -1 ) col = ds.get_cursor_col();

#ifdef DEBUG
      assert( row >= 0 );
      assert( col >= 0 );
      assert( row < ds.get_height() );
      assert( col < ds.get_width() );
#endif

      return &rows[ row ].cells[ col ];
    }

    Cell *get_combining_cell( void );

    void apply_renditions_to_current_cell( void );

    void insert_line( int before_row );
    void delete_line( int row );

    void insert_cell( int row, int col );
    void delete_cell( int row, int col );

    void reset( void );
    void soft_reset( void );

    void set_window_title( std::vector<wchar_t> s ) { window_title = s; }
    std::vector<wchar_t> get_window_title( void ) { return window_title; }

    void resize( int s_width, int s_height );

    void reset_cell( Cell *c ) { c->reset( ds.get_background_rendition() ); }
    void reset_row( Row *r ) { r->reset( ds.get_background_rendition() ); }
  };
}

#endif
