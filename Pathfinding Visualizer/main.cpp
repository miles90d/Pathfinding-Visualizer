#include <iostream>
#include <SDL.h>
#include <deque>
#include <map>
#include "Grid.h"
#include <limits>
#define FPS 60

#define WINDOW_WIDTH 900
#define WINDOW_HEIGHT WINDOW_WIDTH

#define GRID_ROWS 50
void ReconstructPath(SDL_Renderer* renderer, Grid::GridType& grid, int rows, int window_width, std::map<Grid::Node*, Grid::Node*> came_from, Grid::Node* current) {
	while (came_from.find(current) != came_from.end()) {
		current = came_from[current];
		current->SetState(Grid::Node::NODE_STATE_PATH);
		Grid::RenderGrid(renderer, grid, rows, window_width);

	}
}
int H(const Grid::GridPos& pos1, const Grid::GridPos& pos2) {
	return abs(pos1.row - pos2.row) + abs(pos1.col - pos2.col);
}

int main(int argc, char **argv) {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		return -1;
	}
	
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;

	if (SDL_CreateWindowAndRenderer(WINDOW_WIDTH + 1, WINDOW_HEIGHT + 1, SDL_WINDOW_SHOWN, &window, &renderer)) {
		SDL_Log("Unable to create window and renderer: %s", SDL_GetError());
		return -2;
	}
	
	Grid::GridType grid = std::move(Grid::CreateGrid(GRID_ROWS, WINDOW_WIDTH));

	uint32_t starting_tick;
	SDL_Event event;

	bool running = true;

	bool started_pathfinding = false;
	bool resetted = true;
	bool created_start = false;
	bool created_end = false;

	bool mbd = false; //mouse button down
	
	Grid::Node* start_node = nullptr;
	Grid::Node* end_node = nullptr;
	Grid::Node* clicked_node = nullptr;


	while (running) {
		starting_tick = SDL_GetTicks();
		
		Grid::RenderGrid(renderer, grid, GRID_ROWS, WINDOW_WIDTH);
		SDL_RenderPresent(renderer);

		if (mbd) {
			Grid::GridPos clicked_pos = Grid::GetClickedGridPos(event.motion.x, event.motion.y, GRID_ROWS, WINDOW_WIDTH);
			clicked_node = &grid[clicked_pos.row][clicked_pos.col];


			if (event.button.button == SDL_BUTTON_LEFT) {
				if (!created_start) {
					start_node = clicked_node;
					clicked_node->SetState(Grid::Node::NODE_STATE_START);
					created_start = true;
				}
				else if (!created_end && clicked_node->GetState() != Grid::Node::NODE_STATE_START) {
					end_node = clicked_node;
					clicked_node->SetState(Grid::Node::NODE_STATE_END);
					created_end = true;
				}
				else if (clicked_node->GetState() != Grid::Node::NODE_STATE_START
					&& clicked_node->GetState() != Grid::Node::NODE_STATE_END) {
					clicked_node->SetState(Grid::Node::NODE_STATE_OBSTACLE);
				}

			}
			else if (event.button.button == SDL_BUTTON_RIGHT) {
				clicked_node->SetState(Grid::Node::NODE_STATE_DEFAULT);
				if (clicked_node == start_node) {
					start_node = nullptr;
					created_start = false;
				}
				else if (clicked_node == end_node) {
					end_node = nullptr;
					created_end = false;
				}
			}
		}

		while (SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_QUIT:
					running = false;
					break;
				case SDL_MOUSEBUTTONDOWN:
					mbd = true;
					break;
				case SDL_MOUSEBUTTONUP:
					mbd = false;
					break;
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym) {
						case SDLK_SPACE:
							if (!started_pathfinding && created_start && created_end && resetted) {
								started_pathfinding = true;
								//PATHFINDING ALGO START
								for (auto& row : grid) {
									for (auto& node : row) {
										node.UpdateNeighbors(grid);
									}
								}

								
								std::deque<Grid::Node*> open_set;
								open_set.push_front(start_node);
								std::map<Grid::Node*, Grid::Node*> came_from;
								
								std::map<Grid::Node*, float> f_score;
								std::map<Grid::Node*, float> g_score;

								Grid::Node* current_node;

								for (auto& row : grid) {
									for (auto& node : row) {
										g_score[&node] = std::numeric_limits<float>::infinity();
										f_score[&node] = std::numeric_limits<float>::infinity();
									}
								}

								g_score[start_node] = 0;
								f_score[start_node] = H(start_node->GetGridPos(), end_node->GetGridPos());

								while (!open_set.empty()) {
									current_node = open_set.front();
									open_set.pop_front();
									if (current_node == end_node) {
										ReconstructPath(renderer, grid, GRID_ROWS, WINDOW_WIDTH, came_from, end_node);
										break;
									}

									for(auto neighbor: current_node->GetNeighbors()){
										float tentative_g_score = g_score[current_node] + 1;
										if (tentative_g_score < g_score[neighbor]) {
											came_from[neighbor] = current_node;
											g_score[neighbor] = tentative_g_score;
											f_score[neighbor] = g_score[neighbor] + H(neighbor->GetGridPos(), end_node->GetGridPos());
											if (std::find(open_set.begin(), open_set.end(), neighbor) == open_set.end()) {
												open_set.push_back(neighbor);
												neighbor->SetState(Grid::Node::NODE_STATE_OPEN);
											}
										}
									}

									Grid::RenderGrid(renderer, grid, GRID_ROWS, WINDOW_WIDTH);

									if (current_node != start_node){
										current_node->SetState(Grid::Node::NODE_STATE_CLOSED);
									}

								}

								started_pathfinding = false;
								resetted = false;
								//PATHFINDING ALGO END
							} 
							break;
						case SDLK_c:
							resetted = true;
							for (auto& row : grid) {
								for (auto& node : row) {
									node.SetState(Grid::Node::NODE_STATE_DEFAULT);
									node.ResetNeighbors();
								}
							}

							created_start = false;
							created_end = false;
							start_node = nullptr;
							end_node = nullptr;
							break;
					}
					break;
			}
		}
		if ((1000 / FPS) > SDL_GetTicks() - starting_tick)
			SDL_Delay(1000 / FPS - (SDL_GetTicks() - starting_tick));
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;



}