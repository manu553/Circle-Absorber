#include "pebble.h"
#include "stdlib.h"
#include "disc_head.h"
#include "round_math.h"

#define MATH_PI 3.141592653589793238462
#define NUM_DISCS 1
#define DISC_DENSITY 0.25
#define ACCEL_RATIO 0.05
#define ACCEL_STEP_MS 50

/* 

***Possible Game Modes***
    - Time attack: 30 second timer in the top right corner, bombs spawing every 5 seconds (may have to adjust) try to score the highest within those 30 seconds
    - 3 Lives: probably have 3 little icons in the top right, bombs every 5 seconds, hit 3 bombs and game over
    - hardcore: no timer, bombs every 3 seconds, hit a bomb and insta death game over

*** Work to be done ***

    **IN PROGRESS**
        - give the bomb functionality (ie. if they hit the bomb, game over, maybe life system later?)
            - well, the score gets reset to 0 when you hit the bomb and the bomb does move to a different location but the actual
            image of the bomb dosen't move, fine if were doing a 1 life system, not okay for more than 1 life game
            - for multiple life system, in the hit function, I would need to destroy the old bitmap layer, create a new one
            and then spawn the bomb (GBitmap) on the new layer
            
    **INCOMPLETE**
    - find a way to spawn more bombs as time goes by (maybe every 8 seconds spawn a new bomb on screen, play with the timing)
    - MAKE SURE BOMB DOES NOT SPAWN ON TOP OF POINT DOT!
    - program the buttons for the gameover window to restart the game or quit (change later to menu)
    

    **COMPLETED**
        - figure out how to log and debug while the game is running (done)
        - convert the int coord to a string to show where the circle is on the screen (probably make my own function) (done)
        - adjust and finish hit function (done)
        - add counter to one of the layers (maybe main window?) to keep track of
        the number of dots collected (put in top left corner) (done, counter working and function)
        - once counter is displayed make sure the hit function actually works (done, working)
        - if it does work and count goes up, erase the existing dot and RANDOMLY draw a new dot somewhere (done, randomly draws)
        - find a way to randomly draw a new dot (% operator maybe?) (done, rand plus % operator used to keep dot spawing on screen)
        - figure out all this bitmap image stuff (done)
        - maybe add in a bomb icon instead of more default shapes ? (done)
        - add the bomb image (done)
        - game over sceen (done, but need to fix up bomb hit coords)
            - figure out how to create a new window and switch to it (done)
            - get that new window to load once a bomb is hit (done)
            - new window will have a simple text layer that says game over (done, some lag when the bomb is hit, not sure why)
    */

static Window *s_main_window;
static Window *s_gameover_window;
static Layer *s_disc_layer;
static Layer *s_point_layer;
static BitmapLayer *bitmap_layer;
static GBitmap *s_bitmap;
static TextLayer *score_layer;
static TextLayer *gameover_textlayer;
static TextLayer *restart_textlayer;
static TextLayer *quit_textlayer;
static GPoint spot;
static GPoint bombSpot;

static int scorekeeper = 0;

static Disc disc_array[NUM_DISCS];
static GRect window_frame;

char* ftoS(char* buffer, int bufferSize, double number)
{
    char decimalBuffer[5];

    snprintf(buffer, bufferSize, "%d", (int)number);
    strcat(buffer, ".");

    snprintf(decimalBuffer, 5, "%02d", (int)((double)(number - (int)number) * (double)100));
    strcat(buffer, decimalBuffer);

    return buffer;
}

/*
 * This function will handle when the player hits a bomb
 * it should (doesn't yet, ignore that DOES NOW) simply display a new window with a 
 * text layer that says GAME OVER in the middle and has arrows
 * pointing to the top and bottom buttons that will say
 * "main menu" & "restart" respectivly. Not sure if the actual
 * button clicks will be handled in this function.
 *
 *
*/
static void gameOver()
{
    s_gameover_window = window_create();
    Layer *gameover_main = window_get_root_layer(s_gameover_window);    
    
    gameover_textlayer = text_layer_create(GRect(40, 65, 80, 50));
    restart_textlayer = text_layer_create(GRect(70, 15, 80, 30));
    quit_textlayer = text_layer_create(GRect(70, 140, 80, 30));
    static char gameover[30];
    snprintf(gameover, 29, "Game Over!\nFinal Score: %d", scorekeeper);
    text_layer_set_text(gameover_textlayer, gameover);
    layer_add_child(gameover_main, text_layer_get_layer(gameover_textlayer));
    
    text_layer_set_text(restart_textlayer, "Restart--->");
    layer_add_child(gameover_main, text_layer_get_layer(restart_textlayer));
    
    text_layer_set_text(quit_textlayer, "   Quit--->");
    layer_add_child(gameover_main, text_layer_get_layer(quit_textlayer));
    
    window_stack_push(s_gameover_window, true);
}

/*
I believe the hit for the bomb is off because the bomb is being drawn at a specific point aka (1,1) and then the graphic is placed over top kinda like
a start at one corner and then spread out, theory based off of logs and tracking the obsorbers position.

EDIT: fixed for now, adjust the offset to account for the origin position of the bomb + the size of the graphic (16x16 pixels)
*/
static bool hit(GPoint *p, Disc *disc, int ind)
{
    //version 1 - works but need exact point, hard to get and disk doesnt hit every point need to round!
    //if(p->x == (int) disk->pos.x && p->y == (int) disk->pos.y)
    //{
    //    return true;
    //}
    //return false;
    
    int xDistance = (int) disc->pos.x - p->x;
    int yDistance = (int) disc->pos.y - p->y;
    
    if(abs(xDistance) < 5 && abs(yDistance) < 5)
    {
        if(ind == 0)
        {
            spot.x = rand() % (130 - 5 + 1) + 5;
            spot.y = rand() % (130 - 5 + 1) + 5;
            return true;
        }
    }
    
    if(abs(xDistance) < 22 && abs(yDistance) < 22)
    {
        // game over
        if(ind == 1)
        {
            /*
             * Was trying to take the bomb spot and randomly redraw it to a
             * new location, works as the bomb spot moves but the actual image
             * doesn't since its being drawn on a different layer
            bombSpot.x = rand() % (130 - 5 + 1) + 5;
            bombSpot.y = rand() % (130 - 5 + 1) + 5;
            bitmap_layer_destroy(bitmap_layer);
            bitmap_layer = bitmap_layer_create(GRect(bombSpot.x, bombSpot.y, 16, 16));
            bitmap_layer_set_bitmap(bitmap_layer, s_bitmap);
            */
            return true;
        }
    }
    return false;
}

static double disc_calc_mass(Disc *disc)
{
    return MATH_PI * disc->radius * disc->radius * DISC_DENSITY;
}

static void disc_init(Disc *disc)
{
    static double next_radius = 10;

    GRect frame = window_frame;
    disc->pos.x = frame.size.w / 2;
    disc->pos.y = frame.size.h / 2;
    disc->vel.x = 0;
    disc->vel.y = 0;
    disc->radius = next_radius;
    disc->mass = disc_calc_mass(disc);
    #ifdef PBL_COLOR
    disc->color = GColorFromRGB(rand() % 255, rand() % 255, rand() % 255);
    #endif
    //next_radius += 0.5;
}//buffalo

static void disc_apply_force(Disc *disc, Vec2d force) {
  disc->vel.x += force.x / disc->mass;
  disc->vel.y += force.y / disc->mass;
}

static void disc_apply_accel(Disc *disc, AccelData accel) {
  disc_apply_force(disc, (Vec2d) {
    .x = accel.x * ACCEL_RATIO,
    .y = -accel.y * ACCEL_RATIO
  });
}

static void disc_update(Disc *disc) {
    double e = PBL_IF_ROUND_ELSE(0.7, 0.5);

    // update disc position
    disc->pos.x += disc->vel.x;
    disc->pos.y += disc->vel.y;
    
    char xpos[5];
    char ypos[5];
    
    // check if disk has hit point
    if((hit(&spot, disc, 0)) == true)
    {
        scorekeeper++;
    }
    
    // check if disk has hit the bomb some lag when this hits true, not sure why
    if((hit(&bombSpot, disc, 1)) == true)
    {
        gameOver();
    }

#ifdef PBL_ROUND
  // -1 accounts for how pixels are drawn onto the screen. Pebble round has a 180x180 pixel screen.
  // Since this is an even number, the centre of the screen is a line separating two side by side
  // pixels. Thus, if you were to draw a pixel at (90, 90), it would show up on the bottom right
  // pixel from the center point of the screen.
  Vec2d circle_center = (Vec2d) { .x = window_frame.size.w / 2 - 1,
                                  .y = window_frame.size.h / 2 - 1 };

  if ((square(circle_center.x - disc->pos.x) + square(circle_center.y - disc->pos.y)) > square(circle_center.x - disc->radius)) {
    // Check to see whether disc is within screen radius
    Vec2d norm = subtract(disc->pos, circle_center);
    if (get_length(norm) > (circle_center.x - disc->radius)) {
      norm = set_length(norm, (circle_center.x - disc->radius), get_length(norm));
      disc->pos = add(circle_center, norm);
    }
    disc->vel = multiply(find_reflection_velocity(circle_center, disc), e);
  }
#else
  if ((disc->pos.x - disc->radius < 0 && disc->vel.x < 0)
    || (disc->pos.x + disc->radius > window_frame.size.w && disc->vel.x > 0)) {
    disc->vel.x = -disc->vel.x * e;
  }

  if ((disc->pos.y - disc->radius < 0 && disc->vel.y < 0)
    || (disc->pos.y + disc->radius > window_frame.size.h && disc->vel.y > 0)) {
    disc->vel.y = -disc->vel.y * e;
  }
#endif
    
    ftoS(xpos, 4, disc->pos.x);
    ftoS(ypos, 4, disc->pos.y);
    
    APP_LOG(APP_LOG_LEVEL_DEBUG, "X coord for circle: %d", (int) disc->pos.x);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Y coord for cirlce: %d", (int) disc->pos.y);
    
    APP_LOG(APP_LOG_LEVEL_DEBUG, "X coord for score: %d", spot.x);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Y coord for score: %d", spot.y);
    
    APP_LOG(APP_LOG_LEVEL_DEBUG, "X coord for bomb: %d", bombSpot.x);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Y coord for bomb: %d", bombSpot.y);
    
    static char score[13];
    snprintf(score, 12, "Score: %d", scorekeeper);
    text_layer_set_text(score_layer, score);
}

static void disc_draw(GContext *ctx, Disc *disc) 
{
    graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(disc->color, GColorWhite));
    graphics_fill_circle(ctx, GPoint(disc->pos.x, disc->pos.y), disc->radius);
}

static void disc_layer_update_callback(Layer *me, GContext *ctx) 
{
    for (int i = 0; i < NUM_DISCS; i++) 
    {
        disc_draw(ctx, &disc_array[i]);
    }
}

static void timer_callback(void *data) {
  AccelData accel = (AccelData) { .x = 0, .y = 0, .z = 0 };
  accel_service_peek(&accel);

  for (int i = 0; i < NUM_DISCS; i++) 
  {
      Disc *disc = &disc_array[i];
      disc_apply_accel(disc, accel);
      disc_update(disc);
  }

  layer_mark_dirty(s_disc_layer);

  app_timer_register(ACCEL_STEP_MS, timer_callback, NULL);
}

static void s_point_layer_update_callback(Layer *point, GContext *ctx)
{
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_circle(ctx, spot, 5);
}

static void main_window_load(Window *window) 
{
    Layer *window_layer = window_get_root_layer(window);
    GRect frame = window_frame = layer_get_frame(window_layer);

    s_disc_layer = layer_create(frame);
    s_point_layer = layer_create(frame);
    score_layer = text_layer_create(GRect(0, 0, 60, 20));
    bitmap_layer = bitmap_layer_create(GRect(bombSpot.x, bombSpot.y, 16, 16)); // creating bitmap layer where with coords and size for the Gbitmap
    s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BOMB); // assigning the bomb image to s_bitmap (which is a Gbitmap type)
    
    layer_set_update_proc(s_disc_layer, disc_layer_update_callback);
    text_layer_set_background_color(score_layer, GColorBlack);
    text_layer_set_text_color(score_layer, GColorWhite);
    text_layer_set_text_alignment(score_layer, GTextAlignmentLeft);
    
    bitmap_layer_set_background_color(bitmap_layer, GColorBlack);
    bitmap_layer_set_compositing_mode(bitmap_layer, GCompOpSet);
    bitmap_layer_set_bitmap(bitmap_layer, s_bitmap); // putting the bomb Gbitmap onto the bitmap layer (could this set )
    
    layer_set_update_proc(s_point_layer, s_point_layer_update_callback);
    
    
    layer_add_child(window_layer, text_layer_get_layer(score_layer));
    layer_add_child(window_layer, bitmap_layer_get_layer(bitmap_layer));
    layer_add_child(window_layer, s_disc_layer);
    layer_add_child(window_layer, s_point_layer);

    for (int i = 0; i < NUM_DISCS; i++) 
    {
        disc_init(&disc_array[i]);
    }
}

static void main_window_unload(Window *window) {
    layer_destroy(s_disc_layer);
    layer_destroy(s_point_layer);
    text_layer_destroy(score_layer);
    gbitmap_destroy(s_bitmap);
    bitmap_layer_destroy(bitmap_layer);
}

static void init(void) 
{
    // initial point spot
    spot.x = rand() % (130 - 5 + 1) + 5;
    spot.y = rand() % (130 - 5 + 1) + 5;
    
    // initial bomb spot
    /* 
    for some reason the bomb won't update the position, works fine for the spot but not bombspot
    find what's different between spot and bomb spot and we have a functional game
    
    EDIT: setup the debug in the logs and found that the bombspot position is actually moving
    but the actual graphic isn't being updated. need to figure out how to delete the graphic and then respawn it into the new bombspot
    */ 
    bombSpot.x = rand() % (130 - 5 + 1) + 5;
    bombSpot.y = rand() % (130 - 5 + 1) + 5;
    
    s_main_window = window_create();
    window_set_background_color(s_main_window, GColorBlack);
    
    window_set_window_handlers(s_main_window, (WindowHandlers)
    {
      .load = main_window_load,
      .unload = main_window_unload
    });
    
    window_stack_push(s_main_window, true);
    
    accel_data_service_subscribe(0, NULL);

    app_timer_register(ACCEL_STEP_MS, timer_callback, NULL);
}

static void deinit(void) {
  accel_data_service_unsubscribe();

  window_destroy(s_main_window);
  window_destroy(s_gameover_window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
