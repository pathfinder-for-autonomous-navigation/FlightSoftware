/**
 * @author Tanishq Aggarwal (ta335@cornell.edu)
 * @brief Simple utility for testing out vector and quaternion serializers.
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include <common/Serializer.hpp>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <gnc/constants.hpp>
#include <future>

template<typename T, size_t N>
lin::Vector<T, N> to_linvec(const std::array<T, N>& src) {
    lin::Vector<T, N> temp;
    for(unsigned int i = 0; i < N; i++) temp(i) = src[i];
    return temp;
}

/**
 * @brief Returns the magnitude of a vector
 */
template <typename T, size_t N>
T magnitude_of(const lin::Vector<T, N>& src){
    return lin::norm(src);
}
template <typename T, size_t N>
T magnitude_of(const std::array<T, N>& src){
    return magnitude_of(to_linvec(src));
}

/**
 * @brief Normalizes a vector or quaternion
 * 
 */
template <typename T, size_t N>
void normalize(std::array<T, N>& src) {
    T magnitude = magnitude_of(src);
    for(size_t i = 0; i<N; i++){
        src[i] = src[i] / magnitude;
    }
    return;
}
template <typename T, size_t N>
void normalize(lin::Vector<T, N>& src) {
    T magnitude = magnitude_of(src);
    src = src / magnitude;
    return;
}

/**
 * @brief Returns the angle between two quaternions or vectors, in degrees.
 */
template <typename T, size_t N>
T angle_between(lin::Vector<T, N>& a, lin::Vector<T, N>& b)
{
    static_assert(N == 3 || N == 4, "Can only compute angle between vectors and quaternions.");

    T inner_product = lin::dot(a, b);
    
    T angle;
    if(N == 4) {
        // to account for quat could be flipped
        inner_product = std::abs(inner_product);
        angle = std::acos(inner_product)*2.0;
    }
    else
        angle = std::acos(inner_product);

    angle = angle * 360.0 / (2 * 3.14159265);

    return angle;
}

template <typename T, size_t N>
T angle_between(std::array<T, N>& a, std::array<T, N>& b)
{
    lin::Vector<T, N> lin_a;
    lin::Vector<T, N> lin_b;

    if(N == 4){
        lin_a = {a[0], a[1], a[2], a[3]};
        lin_b = {b[0], b[1], b[2], b[3]};
    }
    else{
        lin_a = {a[0], a[1], a[2]};
        lin_b = {b[0], b[1], b[2]};
    }

    return angle_between<T, N>(lin_a, lin_b);
}

template<typename T>
void get_q_results(const std::string& outfile)
{
    // Discretize quaternions by axis-angle representation
    // theta and phi for axis, alpha for angle
    unsigned int ntheta, nphi, nalpha;
    ntheta = 180;
    nphi = 360;
    nalpha = 180; // Alpha limited to 180 degrees to prevent double-cover of SO(3)

    std::ofstream results(outfile, std::ofstream::out);

    Serializer<std::array<T, 4>> s;

    for(int i = 0; i < ntheta; i++) {
        T theta = i * (gnc::constant::pi) / ntheta;

        // Print progress every so often 
        if (i % 9 == 0)
        {
            float percent_progress = i / static_cast<float>(ntheta) * 100;
            std::cout << percent_progress << "%..." << std::endl;
        }

    for(int j = 0; j < nphi; j++) {
        T phi = j * (2 * gnc::constant::pi) / nphi;
        // Parallelize computation of quaternion serializations.

        auto compute_serializations = [theta, phi, nalpha, &s](int alpha_start, int alpha_end)
        {
            std::stringstream alpha_results;
            for(int k = alpha_start; k < alpha_end; k++) {                
                T alpha = k * (2 * gnc::constant::pi) / nalpha;

                lin::Vector<T, 3> n{std::sin(theta) * std::cos(phi), std::sin(theta) * std::sin(phi), std::cos(theta)};
                n = n * std::sin(alpha/2);

                std::array<T, 4> q{n(0), n(1), n(2), std::cos(alpha/2)};
                normalize(q);
                s.serialize(q);
                std::array<T, 4> q_p;
                s.deserialize(&q_p);
                normalize(q_p);
                T q_err = angle_between(q, q_p);
                alpha_results << theta << "," << phi << "," << alpha << "," << q_err << std::endl;
            }
            return alpha_results.str();
        };

        int nthreads = 10;
        std::vector<std::shared_future<std::string>> promises;
        for(int t = 0; t < nthreads; t++)
        {
            int alpha_range = nalpha / nthreads;
            int alpha_start = t * alpha_range;
            int alpha_end = alpha_range + alpha_start;

            promises.emplace_back(std::async(compute_serializations, alpha_start, alpha_end));
        }
        for(auto& promise : promises) results << promise.get();
    }
    }

    results.close();
}

template<typename T>
void get_v_results(const std::string& outfile, T min, T max, unsigned int bitsize)
{
    int ntheta = 180;
    int nphi = 360;
    int nradius = 100;

    std::ofstream results(outfile, std::ofstream::out);

    for(int i = 0; i < ntheta; i++) {
        T theta = i * (gnc::constant::pi) / ntheta;

        // Print progress every so often 
        if (i % 9 == 0)
        {
            float percent_progress = i / static_cast<float>(ntheta) * 100;
            std::cout << percent_progress << "%..." << std::endl;
        }

    for(int j = 0; j < nphi; j++) {
        T phi = j * (2 * gnc::constant::pi) / nphi;

        // Parallelize computation of vector serializations
        auto compute_serializations = [&](int radius_start, int radius_end)
        {
            std::stringstream radius_results;
            T d_radius = (max - min) / nradius;
            for(int k = radius_start; k < radius_end; k++) {
                T radius = min + k * d_radius;

                lin::Vector<T, 3> r = 
                    lin::Vector<T, 3>{std::sin(theta) * std::cos(phi), std::sin(theta) * std::sin(phi), std::cos(theta)} 
                    * radius;
                
                Serializer<lin::Vector<T, 3>> s(min, max, bitsize);
                s.serialize(r);
                lin::Vector<T, 3> r_p;
                s.deserialize(&r_p);
                T dist = lin::norm(r - r_p);
                radius_results << r(0) << "," << r(1) << "," << r(2) << "," << dist << std::endl;
            }
            return radius_results.str();
        };

        int nthreads = 10;
        std::vector<std::shared_future<std::string>> promises;
        for(int t = 0; t < nthreads; t++)
        {
            int radius_range = nradius / nthreads;
            int radius_start = t * radius_range;
            int radius_end = radius_range + radius_start;

            promises.emplace_back(std::async(compute_serializations, radius_start, radius_end));
        }
        for(auto& promise : promises) results << promise.get();
    }
    }

    results.close();
}

// Serialize a vector with the given type
int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cout << "Need to specify output file." << std::endl;
        return 1;
    }

    std::string out = argv[1];
    get_v_results<double>(out, 6841000, 6901000, 26);

    return 0;
}
