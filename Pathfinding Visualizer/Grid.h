#pragma once
#include <vector>
#include <SDL.h>
namespace Grid {
	class Node;
	using GridType = std::vector<std::vector<Node>>;
	struct GridPos {
		int row, col;
	};

	SDL_Color DEFAULT_COLOR = { 22, 22, 22, 255 };
	SDL_Color OPEN_COLOR = { 204, 255, 153, 255 };
	SDL_Color CLOSED_COLOR = { 255, 102, 102, 255 };
	SDL_Color OBSTACLE_COLOR = { 0, 0, 0, 255 };
	SDL_Color START_COLOR = { 255, 153, 51, 255 };
	SDL_Color END_COLOR = { 0, 153, 153, 255 };
	SDL_Color PATH_COLOR = { 255, 0, 255, 255 };

	SDL_Color LINE_COLOR = { 44, 44, 44, 255 };

	class Node {
	public:
		enum NodeState {
			NODE_STATE_DEFAULT = 0,
			NODE_STATE_OPEN,
			NODE_STATE_CLOSED,
			NODE_STATE_OBSTACLE,
			NODE_STATE_START,
			NODE_STATE_END,
			NODE_STATE_PATH
		};
	private:
		NodeState state_;
		SDL_Rect rect_; //container for pos and width
		int total_rows_;
		GridPos grid_pos_;
		std::vector<Node*>  neighbors_;
	public:
		Node(GridPos grid_pos, int grid_size, int total_rows)
			: grid_pos_(grid_pos), total_rows_(total_rows), state_(NODE_STATE_DEFAULT) {
			rect_.x = grid_pos_.col * grid_size;
			rect_.y = grid_pos_.row * grid_size;
			rect_.w = rect_.h = grid_size;
		}

		GridPos GetGridPos() const {
			return grid_pos_;
		}

		std::vector<Node*> GetNeighbors() const {
			return neighbors_;
		}

		void ResetNeighbors() {
			neighbors_.clear();
		}


		void Render(SDL_Renderer* renderer) const {
			switch (state_) {
			case NODE_STATE_DEFAULT:
				SDL_SetRenderDrawColor(renderer, DEFAULT_COLOR.r, DEFAULT_COLOR.g, DEFAULT_COLOR.b, DEFAULT_COLOR.a);
				break;
			case NODE_STATE_OPEN:
				SDL_SetRenderDrawColor(renderer, OPEN_COLOR.r, OPEN_COLOR.g, OPEN_COLOR.b, OPEN_COLOR.a);
				break;
			case NODE_STATE_CLOSED:
				SDL_SetRenderDrawColor(renderer, CLOSED_COLOR.r, CLOSED_COLOR.g, CLOSED_COLOR.b, CLOSED_COLOR.a);
				break;
			case NODE_STATE_OBSTACLE:
				SDL_SetRenderDrawColor(renderer, OBSTACLE_COLOR.r, OBSTACLE_COLOR.g, OBSTACLE_COLOR.b, OBSTACLE_COLOR.a);
				break;
			case NODE_STATE_START:
				SDL_SetRenderDrawColor(renderer, START_COLOR.r, START_COLOR.g, START_COLOR.b, START_COLOR.a);
				break;
			case NODE_STATE_END:
				SDL_SetRenderDrawColor(renderer, END_COLOR.r, END_COLOR.g, END_COLOR.b, END_COLOR.a);
				break;
			case NODE_STATE_PATH:
				SDL_SetRenderDrawColor(renderer, PATH_COLOR.r, PATH_COLOR.g, PATH_COLOR.b, PATH_COLOR.a);
				break;
			}

			SDL_RenderFillRect(renderer, &rect_);
		}

		void SetState(const NodeState& state) {
			state_ = state;
		}

		NodeState GetState() const {
			return state_;
		}

		void UpdateNeighbors(GridType& grid) {
			if (grid_pos_.row < total_rows_ - 1 &&
				grid[grid_pos_.row + 1][grid_pos_.col].GetState() != NODE_STATE_OBSTACLE) {
				neighbors_.push_back(&grid[grid_pos_.row + 1][grid_pos_.col]);
			}

			if (grid_pos_.row > 0 &&
				grid[grid_pos_.row - 1][grid_pos_.col].GetState() != NODE_STATE_OBSTACLE) {
				neighbors_.push_back(&grid[grid_pos_.row - 1][grid_pos_.col]);
			}

			if (grid_pos_.col < total_rows_ - 1 &&
				grid[grid_pos_.row][grid_pos_.col + 1].GetState() != NODE_STATE_OBSTACLE) {
				neighbors_.push_back(&grid[grid_pos_.row][grid_pos_.col + 1]);
			}

			if (grid_pos_.col > 0 &&
				grid[grid_pos_.row][grid_pos_.col - 1].GetState() != NODE_STATE_OBSTACLE) {
				neighbors_.push_back(&grid[grid_pos_.row][grid_pos_.col - 1]);
			}
			
		}

		bool operator<(const Node& other) {
			return false;
		}

	};

	GridType CreateGrid(const int& rows, const int& window_width) {
		GridType grid;
		grid.reserve(rows);
		int grid_size = window_width / rows;
		GridPos grid_pos;

		for (int i = 0; i < rows; i++) {
			std::vector<Node> row;
			for (int j = 0; j < rows; j++) {
				grid_pos.row = i;
				grid_pos.col = j;
				row.emplace_back(grid_pos, grid_size, rows);
			}
			grid.push_back(row);
		}

		return grid;
	}

	void RenderGridLines(SDL_Renderer* renderer, const int& rows, const int& window_width) {
		int grid_size = window_width / rows;
		SDL_SetRenderDrawColor(renderer, LINE_COLOR.r, LINE_COLOR.g, LINE_COLOR.b, LINE_COLOR.a);
		for (int x = 0; x < 1 + grid_size * rows; x += grid_size) {
			SDL_RenderDrawLine(renderer, x, 0, x, (rows * grid_size) + 1);
		}

		for(int y = 0; y < 1 + grid_size * rows; y+= grid_size){
			SDL_RenderDrawLine(renderer, 0, y, (rows * grid_size) + 1, y);
		}
	}

	void RenderGrid(SDL_Renderer *renderer, const GridType& grid, const int& rows, const int& window_width) {
		for (auto& row : grid) {
			for (const auto& node : row) {
				node.Render(renderer);
			}
		}

		RenderGridLines(renderer, rows, window_width);
		SDL_RenderPresent(renderer);
	}

	GridPos GetClickedGridPos(int mx, int my, int rows, int window_width) {
		int grid_size = window_width / rows;
		GridPos pos;
		pos.row = my / grid_size;
		pos.col = mx / grid_size;
		
		return pos;
	}


}
