#include <thread>
#include <cmath>
#include <vector>

template <typename T>
class Effect {
  public:
    virtual ~Effect() {}
    virtual void apply() = 0;
    virtual T& get() = 0;
};

template <typename T>
class Blur: public Effect<T> {
  public:
    Blur(T& in, int kernel_size): in_(in), out_(in), ksize_(kernel_size) {}

    /// @brief Apply specific transformation to the input image
    void apply()
    {
        total = 0.0;
        K_.clear();
        auto Hk = get_hk();
        auto sigma_small = 0.3 * ((ksize_ - 1.0) * 0.5 - 1.0) + 0.8;

        for (int kx = -Hk; kx <= Hk; kx++) {
          for (int ky = -Hk; ky <= Hk; ky++) {
            auto k = K(kx, ky, sigma_small);
            K_.push_back(k);
            total += k;
          }
        }

        std::vector<std::thread> threads_;

        static_assert(max_threads_ > 0, "There should be one thread at least");
        int width_per_thread = in_.width / max_threads_;

        for (int i = 0; i < max_threads_; i++)
        {
          std::thread t([&](int i){
            transform(i * width_per_thread, 0, (i+1) * width_per_thread, in_.height);
          }, i);
          threads_.push_back(std::move(t));
        }

        for (auto& t : threads_)
        {
          t.join();
        }
    }

    T& get()
    {
        return out_;
    }

  protected:
    /// @brief Transforms input image
    /// @param start_x image X coordinate to start from
    /// @param start_y image Y coordinate to start from
    /// @param width image height to stop at
    /// @param height image width to stop at
    void transform(unsigned int start_x, unsigned int start_y, unsigned int width, unsigned int height)
    {
        auto Hk = get_hk();

        auto get_pixel = [&](auto x, auto y, decltype(in_) &image)
        {
          static_assert(std::is_same<std::vector<unsigned char>, decltype(image.data)>::value, "Data storage must be a vector of unsigned char");
          return &image.data[(x + y * image.width) * 4];
        };

        for (unsigned int x = start_x; x < width; ++x) {
          for (unsigned int y = start_y; y < height; ++y) {
            auto *pixel_out = get_pixel(x, y, out_);
            std::vector<double> out = {0.0, 0.0, 0.0, 0.0};

            for (int kx = -Hk; kx <= Hk; kx++) {
              for (int ky = -Hk; ky <= Hk; ky++) {
                auto x_ = x + kx;
                auto y_ = y + ky;
                if (is_valid_point(x_, y_)) {
                  auto *pixel_in = get_pixel(x_, y_, in_);
                  for (int i = 0; i < 4; i++) {
                    out[i] += static_cast<double>(pixel_in[i]) * K_[((ky+Hk) + (kx+Hk) * ksize_)];
                  }
                }
              }
            }

            for (int i = 0; i < 4; i++) {
              pixel_out[i] = static_cast<uint8_t>(out[i] / total);
            }
          }
        }
    }

    /// @brief Checks if point is a valid input image point
    /// @param x image X coordinate
    /// @param y image Y coordinate
    /// @return true if point is valid
    auto is_valid_point(auto x, auto y)
    {
      return x >= 0 && y >= 0 && x < in_.width && y < in_.height;
    }

    /// @brief Returns window size
    /// @param x window X coordinate
    /// @param y window Y coordinate
    /// @param sigma_small small sigma
    auto K(int x, int y, double sigma_small)
    {
      return exp(-(pow(x, 2) + pow(y, 2)) / (2 * pow(sigma_small, 2)));
    }

    /// @brief Returns window size
    /// @return integer window size
    auto get_hk()
    {
        return (ksize_ - 1) / 2;
    }

  private:
    T& in_;
    T out_;
    int ksize_ = 0;
    double total = 0.0;
    std::vector<double> K_;

    static constexpr int max_threads_ = 8;
};
