// mandelbrot.cpp
// compile with: g++ -std=c++11 mandelbrot.cpp -o mandelbrot
// view output with: eog mandelbrot.ppm

#include <fstream>
#include <iostream>
#include <thread>
#include <vector>
#include <complex> // if you make use of complex number facilities in C++

template <class T> struct RGB { T r, g, b; };

template <class T>
class Matrix {
public:
    Matrix(const size_t rows, const size_t cols) : _rows(rows), _cols(cols) {
        _matrix = new T*[rows];
        for (size_t i = 0; i < rows; ++i) {
            _matrix[i] = new T[cols];
        }
    }
    Matrix(const Matrix &m) : _rows(m._rows), _cols(m._cols) {
        _matrix = new T*[m._rows];
        for (size_t i = 0; i < m._rows; ++i) {
            _matrix[i] = new T[m._cols];
            for (size_t j = 0; j < m._cols; ++j) {
                _matrix[i][j] = m._matrix[i][j];
            }
        }
    }
    ~Matrix() {
        for (size_t i = 0; i < _rows; ++i) {
            delete [] _matrix[i];
        }
        delete [] _matrix;
    }
    T *operator[] (const size_t nIndex)
    {
        return _matrix[nIndex];
    }
    size_t width() const { return _cols; }
    size_t height() const { return _rows; }
protected:
    size_t _rows, _cols;
    T **_matrix;
};

// Portable PixMap image
class PPMImage : public Matrix<RGB<unsigned char> >
{
public:
    PPMImage(const size_t height, const size_t width) : Matrix(height, width) { }
    void save(const std::string &filename)
    {
        std::ofstream out(filename, std::ios_base::binary);
        out <<"P6" << std::endl << _cols << " " << _rows << std::endl << 255 << std::endl;
        for (size_t y=0; y<_rows; y++)
            for (size_t x=0; x<_cols; x++)
                out << _matrix[y][x].r << _matrix[y][x].g << _matrix[y][x].b;
    }
};

/*
 The function that calculates if a point is within the set or not
 */
int findMandelBrot(double cr, double ci, int max_iterations)
{
    int i = 0;
    double zr = 0.0, zi = 0.0;
    while(i < max_iterations && zr * zr + zi * zi < 4.0)
    {
        double temp = zr * zr - zi * zi + cr;
        zi =2.0 * zr * zi + ci;
        zr = temp;
        i++;
    }

    if(i >= max_iterations)
        return -1;
    return i;
}

/* Function that retrieves the real number*/
double mapToReal (int x, int width, double minR, double maxR)
{
    double range = maxR - minR;

    return x * (range / width) + minR;
}

/* Function that retrieves the imaginary number*/
double mapToImaginary(int y, int height, double minI, double maxI)
{
    double range = maxI - minI;

    return y * (range / height) + minI;
}

/* Function that sets the color for each pixel*/
int * getColor (int n)
{
    int * colors = new int[3];

    if(n == -1)
    {
        colors[0] = colors[1] = colors[2] = 0;
    }
    else if(n == 0)
    {
        colors[0] = 255;
        colors[1] = colors[2] = 0;
    }
    else
    {
        if (n < 16) {
            colors[0] = 16 * (16 - n);
            colors[1] = 0;
            colors[2] = 16 * n - 1;
        } else if (n < 32) {
            colors[0] = 0;
            colors[1] = 16 * (n - 16);
            colors[2] = 16 * (32 - n) - 1;
        } else if (n < 64) {
            colors[0] = 8 * (n - 32);
            colors[1] = 8 * (64 - n) - 1;
            colors[2] = 0;
        } else { // range is 64 - 127
            colors[0] = 255 - (n - 64) * 4;
            colors[1] = colors[2] = 0;
        }
    }

    return colors;
}

/* This function sets the needed variables for the mandelbrot aswell as calculating and drawing it */
void mandelbrot(int minX, int maxX, int width, int height, PPMImage &image){
    const int maxIterations = 127;
    const double minR = -2.0;
    const double maxR = 0.7;
    const double minI = -1.2;
    const double maxI = 1.2;

    for (int i = 0; i < height; i++) //Rows
    {
        for(int j = minX; j < maxX; j++) //Pixels in row
        {
            double cr = mapToReal(j, width, minR, maxR);
            double ci = mapToImaginary(i, height, minI, maxI);

            int n = findMandelBrot(cr, ci, maxIterations);

            int* colors = getColor(n);
            image[i][j].r = colors[0];
            image[i][j].g = colors[1];
            image[i][j].b = colors[2];

            delete colors;

            //This coloring is way faster but less pretty
/*            if(n == -1){
                image[i][j].r = image[i][j].g = image[i][j].b = 0;
            } else{
                image[i][j].r = image[i][j].g = image[i][j].b = 100;
            }*/
        }
    }
}

int main()
    {
    const unsigned width = 1600;
    const unsigned height = 1300;
    const unsigned threads = 4;

    PPMImage image(height, width);

    std::vector<std::thread> workers;

    clock_t start_clock = clock();
    for (int i = 0; i < threads; i++)
    {
        int part = width / threads;
        int minX = part * i;
        int maxX = part * (i+1);

        workers.emplace_back(std::thread([minX, maxX, width, height, &image]()
        {
            mandelbrot(minX, maxX, width, height, std::ref(image));
        }));
    };
    for(auto &th : workers)
    {
        th.join();
    }
    clock_t stop_clock = clock();
    std::cout << (double(stop_clock - start_clock) / CLOCKS_PER_SEC) << " seconds\n";
    image.save("mandelbrot.ppm");
    return 0;
}


