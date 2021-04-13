#include <iostream>
#include <vector>
#include <queue>

#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/imgcodecs.hpp>


struct Pixel {
    Pixel() = default;

    Pixel(int x, int y) : x(x), y(y) {}

    int x = 0;
    int y = 0;

    bool operator==(Pixel other) const {
        return x == other.x && y == other.y;
    }

    bool operator!=(Pixel other) const {
        return !(*this == other);
    }
};

const Pixel BAD_PIXEL(-1, -1);

class Image {
public:
    explicit Image(cv::Mat image);

    size_t GetRows() const {
        return is_white_.size();
    }

    size_t GetColumns() const {
        return is_white_[0].size();
    }

    std::vector<Pixel> GetNeighbours(Pixel pixel) {
        return GetNeighbours(pixel.x, pixel.y);
    }

    std::vector<Pixel> GetNeighbours(int x, int y);

    bool IsWhite(int x, int y) {
        return is_white_[x][y];
    }

    bool IsWhite(Pixel pixel) {
        return IsWhite(pixel.x, pixel.y);
    }

private:
    static bool IsWhite(uchar color) {
        return color == 0;
    }

    bool IsCorrect(int x, int y) {
        return x >= 0 && x < is_white_.size() &&
               y >= 0 && y < is_white_[0].size();
    }

    std::vector<std::vector<uchar>> is_white_;
};

Image::Image(cv::Mat image) : is_white_(image.rows, std::vector<uchar>(image.cols)) {
    for (int i = 0; i < image.rows; i++) {
        for (int j = 0; j < image.cols; j++) {
            uchar color = image.at<uchar>(i, j);
            is_white_[i][j] = IsWhite(color);
        }
    }
}

std::vector<Pixel> Image::GetNeighbours(int x, int y) {
    std::vector<Pixel> neighbours;
    if (IsCorrect(x + 1, y)) {
        neighbours.emplace_back(x + 1, y);
    }
    if (IsCorrect(x, y + 1)) {
        neighbours.emplace_back(x, y + 1);
    }
    if (IsCorrect(x - 1, y)) {
        neighbours.emplace_back(x - 1, y);
    }
    if (IsCorrect(x, y - 1)) {
        neighbours.emplace_back(x, y - 1);
    }
    return neighbours;
}


void BFS(Pixel start, Image& image, std::vector<Pixel>& black_pixels) {
    std::vector<std::vector<char>> used(image.GetRows(), std::vector<char>(image.GetColumns()));
    std::queue<Pixel> queue;
    queue.push(start);
    used[start.x][start.y] = true;

    size_t white = 0;
    size_t black = 1; // start pixel
    const size_t MIN_ITERATION_COUNT = 200;
    const size_t MAX_ITERATION_COUNT = 400;

    while (!queue.empty() && (white / black == 0 || white + black < MIN_ITERATION_COUNT) &&
           black + white < MAX_ITERATION_COUNT) {
        Pixel next = queue.front();
        queue.pop();
        if (!image.IsWhite(next)) {
            black_pixels.push_back(next);
        }
        for (auto p : image.GetNeighbours(next)) {
            if (!used[p.x][p.y]) {
                queue.push(p);
                used[p.x][p.y] = true;
            }
        }
    }
}


Pixel GetLeftest(std::vector<Pixel>& pixels) {
    Pixel best_pixel;
    int min_x = INT_MAX;
    for (auto p : pixels) {
        if (p.x < min_x) {
            best_pixel = p;
            min_x = p.x;
        }
    }
    return best_pixel;
}

Pixel GetRightest(std::vector<Pixel>& pixels) {
    Pixel best_pixel;
    int max_x = INT_MIN;
    for (auto p : pixels) {
        if (max_x < p.x) {
            best_pixel = p;
            max_x = p.x;
        }
    }
    return best_pixel;
}

Pixel GetLowest(std::vector<Pixel>& pixels) {
    Pixel best_pixel;
    int min_y = INT_MIN;
    for (auto p : pixels) {
        if (p.y < min_y) {
            best_pixel = p;
            min_y = p.y;
        }
    }
    return best_pixel;
}

Pixel GetHighest(std::vector<Pixel>& pixels) {
    Pixel best_pixel;
    int max_y = INT_MIN;
    for (auto p : pixels) {
        if (max_y < p.y) {
            best_pixel = p;
            max_y = p.y;
        }
    }
    return best_pixel;
}

bool AreSimilar(int a, int b) {
    return std::abs(a - b) < 5;
}

bool AreSimilar(Pixel a, Pixel b) {
    return AreSimilar(a.x, b.x) && AreSimilar(a.y, b.y);
}

bool IsIntersection(Pixel start, Image& image) {
    std::vector<Pixel> black_pixels;
    BFS(start, image, black_pixels);

    Pixel leftest = GetLeftest(black_pixels);
    Pixel rightest = GetRightest(black_pixels);
    Pixel lowest = GetLowest(black_pixels);
    Pixel highest = GetHighest(black_pixels);

    std::vector<Pixel> external_pixels;
    external_pixels.push_back(leftest);
    external_pixels.push_back(rightest);
    external_pixels.push_back(lowest);
    external_pixels.push_back(highest);

    for (int i = 0; i < 4; i++) {
        for (int j = i + 1; j < 4; j++) {
            if (AreSimilar(external_pixels[i], external_pixels[j])) {
                return false;
            }
        }
    }
    return true;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "You need to give name of png file\n";
        return 1;
    }
    std::string image_path = argv[1];

    Image image(cv::imread(image_path, cv::IMREAD_GRAYSCALE));


    std::vector<Pixel> potential_intersections;
    const int STEP = 5;
    for (int i = 0; i < image.GetRows(); i += STEP) {
        for (int j = 0; j < image.GetColumns(); j += STEP) {
            if (image.IsWhite(i, j)) {
                if (IsIntersection({i, j}, image)) {
                    potential_intersections.emplace_back(i, j);
                    i += 20;
                }
            }
        }
    }
    std::vector<char> is_bad_pixel(potential_intersections.size());
    for (int i = 0; i < potential_intersections.size(); i++) {
        for (int j = i + 1; j < potential_intersections.size(); j++) {
            if (AreSimilar(potential_intersections[i], potential_intersections[j])) {
                is_bad_pixel[j] = true;
            }
        }
    }

    int intersections_count = 0;
    for (auto is_bad : is_bad_pixel) {
        if (!is_bad) {
            intersections_count++;
        }
    }

    std::cout << intersections_count << std::endl;

    return 0;
}
