#include <stddef.h>
#include <vector>
#include <iostream>
#include <math.h>
#include <memory>
#include <mutex>
#include <thread>
#include <cmath>

#include "lodepng.h"
#include "timer.h"
#include "blur.h"

template <typename T>
struct Image {
  Image() = default;

  Image(unsigned int width, unsigned int height) : width(width), height(height) {
    data = std::vector<T>(width * height * 4, 0);
  }

  std::vector<T> data; ///< Data for the image
  unsigned int width; ///< Width of the image
  unsigned int height; ///< Height of the image
};

/// @brief Load image from a png file
/// @param filename Path to png file to load
/// @return Returns the image image
Image<unsigned char> loadPNG(const std::string &filename) {
  std::vector<unsigned char> png;
  Image<unsigned char> image;
  lodepng::State state; //optionally customize this one

  unsigned int error = lodepng::load_file(png, filename); //load the image file with given filename
  if (!error) {
    error = lodepng::decode(image.data, image.width, image.height, state, png);
  }

  //if there's an error, display it
  if (error) {
    std::cout << "decoder error " << error << ": "<< lodepng_error_text(error) << std::endl;
  }

  //the pixels are now in the vector "image", 4 bytes per pixel, ordered RGBARGBA..., use it as texture, draw it, ...
  //State state contains extra information about the PNG such as text chunks, ...
  return image;
}

template <typename T>
class ImageWriter
{
public:
    /// @brief Writes image data to file when object is destroyed
    /// @param filename Image filepath to save at
    /// @param image The image that is going to be saved
    ImageWriter(const std::string filename, const T &image): filename_(filename), image_(image)
    {
    }

    ~ImageWriter()
    {
        savePNG(filename_, image_);
    }

protected:
    void savePNG(const std::string &filename, const Image<unsigned char> &image) {
      std::vector<unsigned char> png;
      lodepng::State state; //optionally customize this one

      unsigned int error = lodepng::encode(png, image.data, image.width, image.height, state);
      if (!error) {
        lodepng::save_file(png, filename.c_str());
      }

      //if there's an error, display it
      if (error) {
        std::cout << "encoder error " << error << ": "<< lodepng_error_text(error) << std::endl;
      }
    }

private:
    const std::string filename_;
    const T &image_;
};

int main(int argc, char *argv[]) {
  if (argc != 4) {
    printf("Usage: ./a test_input.png test_output.png kernel_size\n");
    return 0;
  }

  long int kernel_size = strtol(argv[3], nullptr, 0);
  if (kernel_size % 2 == 0 || kernel_size < 0.0) {
    printf("Kernel size must be odd positive integer\n");
    return 0;
  }

  auto image = loadPNG(argv[1]);
  std::unique_ptr<Effect<decltype(image)>> effect(new Blur<decltype(image)>(image, kernel_size));
  ImageWriter<decltype(image)> writer(argv[2], effect->get());
  Timer timer;

  /// Perform calculations
  effect->apply();
}
