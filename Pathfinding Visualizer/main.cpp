#include <iostream>
#include <SDL.h>
#include <queue>
#include <map>
#include "Grid.h"
#define FPS 60

#define WINDOW_WIDTH 900
#define WINDOW_HEIGHT WINDOW_WIDTH

#define GRID_ROWS 50
void ReconstructPath(SDL_Renderer* renderer, Grid::GridType& grid, int rows, int window_width, std::map<Grid::Node*, Grid::Node*> came_from, Grid::Node* current, Grid::Node* end) {
	while (came_from.find(current) != came_from.find(end)) {
		current->SetState(Grid::Node::NODE_STATE_PATH);
		current = came_from[current];
		Grid::RenderGrid(renderer, grid, rows, window_width);
	}
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
						case SDLK_SPACE: //start pathfinding
							if (!started_pathfinding && created_start && created_end && resetted) {
								started_pathfinding = true;
								//PATHFINDING ALGO START
								std::queue<Grid::Node*> q;
								std::map<Grid::Node*, bool> visited; 
								std::map<Grid::Node*, Grid::Node *> came_from;
								for (auto& row : grid) {
									for (auto& node : row) {
										came_from[&node] = nullptr;
										visited[&node] = false;
										node.UpdateNeighbors(grid);
									}
								}
								
								q.push(start_node);
								
								while (!q.empty()) {
									Grid::Node* curr = q.front();
									q.pop();
									if (curr == end_node) {
										ReconstructPath(renderer, grid, GRID_ROWS, WINDOW_HEIGHT, came_from, end_node, start_node);

										break;
									}

									for (auto neighbor : curr->GetNeighbors()) {
										if (!visited[neighbor] && neighbor->GetState() != Grid::Node::NODE_STATE_OBSTACLE){
											q.push(neighbor);
											neighbor->SetState(Grid::Node::NODE_STATE_OPEN);
											visited[neighbor] = true;
											came_from[neighbor] = curr;
										}
									}
									Grid::RenderGrid(renderer, grid, GRID_ROWS, WINDOW_WIDTH);
									curr->SetState(Grid::Node::NODE_STATE_CLOSED);
								}

								started_pathfinding = false;
								resetted = false;



								
								//PATHFINDING ALGO END
							} 
							break;
						case SDLK_c: //clear grid
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