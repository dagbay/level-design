#include "splashkit.h"
#include "tile.h"
#include "cellsheet.h"
#include <fstream>
#include <vector>

#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 896

//Gives extra size to the screen, for use with non-static camera
//Multiply by tile size of the map
#define SCREEN_WIDTH_EXTRA 64 * 10
#define SCREEN_HEIGHT_EXTRA 64 * 10

void write_out_level_to_file(string file, vector<vector<Tile>> tiles)
{
    std::ofstream level;
    level.open(file);

    for(int i = 0; i < tiles.size(); i++)
    {
        for(int j = 0; j < tiles[i].size(); j ++)
            level << tiles[i][j].get_value() << " ";
        level << "\n";
    }
    level.close();
}

void update_camera(vector_2d mouse_coordinates)
{
    if (key_down(LEFT_ALT_KEY))
    {
        move_camera_by(mouse_coordinates.x * -1, mouse_coordinates.y * -1);
    }  
}

vector<vector<Tile>> make_layer(int tile_size, bitmap initial_bitmap, int extra_width, int extra_height)
{
    vector<vector<Tile>> tiles;
    point_2d origin;
    origin.x = 0;
    origin.y = 0;

    for(int j = 0; j < (SCREEN_HEIGHT + extra_height)/tile_size; j++)
    {
        vector<Tile> row;
        for(int i = 0; i < (SCREEN_WIDTH + extra_width)/tile_size; i++)
        {
            origin.x = 0 + (i * tile_size);
            origin.y = 0 + (j * tile_size);
            Tile test(initial_bitmap, origin, tile_size);
            row.push_back(test);
        }

        tiles.push_back(row);
    }

    return tiles;
}

vector<CellSheet> make_cell_sheets(vector<string> cell_sheet_names)
{
    vector<CellSheet> cell_sheets;

    int offset = 0;
    for(int i = 0; i < cell_sheet_names.size(); i++)
    {
        bitmap new_bitmap = bitmap_named(cell_sheet_names[i]);
        CellSheet new_type(new_bitmap, cell_sheet_names[i], offset);
        offset += new_type.number_of_cells;
        cell_sheets.push_back(new_type);
    }

    return cell_sheets;
}

int main(int argc, char *argv[])
{
    load_resource_bundle("game_resources", "gameresources.txt");
    open_window("Level Design", SCREEN_WIDTH, SCREEN_HEIGHT);
    bool hud = true;

    vector<string> cell_sheet_names;
    
    cell_sheet_names.push_back("Solid");
    cell_sheet_names.push_back("Ladder");
    cell_sheet_names.push_back("Pipe");
    cell_sheet_names.push_back("Water");
    cell_sheet_names.push_back("Toxic");
    cell_sheet_names.push_back("HoldPipes");
    cell_sheet_names.push_back("TurnPipes");
    cell_sheet_names.push_back("Empty");
    
    //Players have a specific offset
    vector<CellSheet> cell_sheets = make_cell_sheets(cell_sheet_names);

    bitmap bitmap_players = bitmap_named("Players");
    CellSheet player_type(bitmap_players, "Players", 908);
    cell_sheets.push_back(player_type);

    bitmap bitmap_door = bitmap_named("Door");
    CellSheet door_type(bitmap_door, "Door", 807);
    cell_sheets.push_back(door_type);

    bitmap bitmap_roach = bitmap_named("Roach");
    CellSheet roach_type(bitmap_roach, "Roach", 706);
    cell_sheets.push_back(roach_type);


    int cell_sheet_selection = 0;

    int layers = 2;
    int tile_size = 64;
    int extra_width =  tile_size * 0;
    int extra_height =  tile_size * 0;
    string file_name = "file";
    

    int tile_selection = 1;
    int current_layer = 0;

    std::vector<std::string> args(argv, argv+argc);

    try
    {
        for (size_t i = 1; i < args.size(); ++i) 
        {
            if (args[i] == "-l") 
                layers = std::stoi(args[i + 1]);

            if (args[i] == "-t") 
                tile_size = std::stoi(args[i + 1]);
            
            if (args[i] == "-xh") 
                extra_height = tile_size * std::stoi(args[i + 1]);

            if (args[i] == "-xw") 
                extra_width = tile_size * std::stoi(args[i + 1]);
            
            if (args[i] == "-f") 
                file_name = args[i + 1];
        }
    }
    catch(const std::exception& e)
    {
        write_line(e.what());
        write_line("Closing program");
        exit(1);
    }

    drawing_options opts = option_defaults();
    opts.draw_cell = 0;
    opts.camera = DRAW_TO_SCREEN;

    vector<vector<vector<Tile>>> all_layers;

    for(int i = 0; i < layers; i++)
    {
        vector<vector<Tile>> tiles = make_layer(tile_size, cell_sheets[cell_sheet_selection].cells, extra_width, extra_height);
        all_layers.push_back(tiles);
    }

    while(!key_typed(ESCAPE_KEY))
    {
        clear_screen(COLOR_BLACK);


        for(int i = 0; i < all_layers[current_layer].size(); i++)
        {
            for(int j = 0; j < all_layers[current_layer][i].size(); j++)
            {
                all_layers[current_layer][i][j].check_input(tile_selection, cell_sheets[cell_sheet_selection].cells, cell_sheets[cell_sheet_selection].offset);
            }
        }
            
        for(int i = 0; i < all_layers.size(); i++)
        {
            for(int j = 0; j < all_layers[i].size(); j++)
            {
                for(int k = 0; k < all_layers[i][j].size(); k++)
                {
                    all_layers[i][j][k].draw_tile();
                }
            }
        }

        if(key_typed(W_KEY))
        {
            if(current_layer < all_layers.size() - 1)
                current_layer += 1;
        }

        if(key_typed(Q_KEY))
        {
            if(current_layer > 0)
                current_layer -= 1;
        }

        if(key_typed(A_KEY))
        {
            for(int i = 0; i < all_layers.size(); i++)
            {
                string file = file_name + std::to_string(i) + ".txt";
                write_out_level_to_file(file, all_layers[i]);
            }
        }

        if(key_typed(H_KEY))
        {
            if(hud)
                hud = false;
            else
                hud = true;
        }

        if(key_typed(NUM_2_KEY))
        {
            if(cell_sheet_selection < cell_sheets.size() - 1)
            {
                cell_sheet_selection += 1;
                tile_selection = 1;
                opts.draw_cell = 0;
            }
        }
        if(key_typed(NUM_1_KEY))
        {
            if(cell_sheet_selection > 0)
            {
                cell_sheet_selection -= 1;
                tile_selection = 1;
                opts.draw_cell = 0;
            }
        }

        if(key_typed(UP_KEY) || mouse_wheel_scroll().y > 0)
        {
            tile_selection += 1;
            opts.draw_cell += 1;

            if(tile_selection == 1)
            {
                opts.draw_cell = 0;
            }

            if(tile_selection > bitmap_cell_count(cell_sheets[cell_sheet_selection].cells))
            {
                tile_selection = bitmap_cell_count(cell_sheets[cell_sheet_selection].cells);
                opts.draw_cell = bitmap_cell_count(cell_sheets[cell_sheet_selection].cells) - 1;
            }
        }

        if(key_typed(DOWN_KEY) || mouse_wheel_scroll().y < 0)
        {
            tile_selection -= 1;
            opts.draw_cell -= 1;

            if(tile_selection == 0 || tile_selection < 0)
            {
                tile_selection = 1;
                opts.draw_cell = 0;
            }
        }

        point_2d current_mouse_position = mouse_position();
        
        update_camera(mouse_movement());

        if(hud)
        {
            draw_text("Current Cell Sheet: " + cell_sheets[cell_sheet_selection].type, COLOR_WHITE, "basicFont", 20, 0, 0, opts);
            draw_text("Current Layer: " + std::to_string(current_layer + 1), COLOR_WHITE, "basicFont", 20, 0, 20, opts);
        }

        if(tile_selection > 0)
            draw_bitmap(cell_sheets[cell_sheet_selection].cells, current_mouse_position.x, current_mouse_position.y, opts);
        
        process_events();
        refresh_screen(60);
    }

    free_resource_bundle("game_resources");
    return 0;
}