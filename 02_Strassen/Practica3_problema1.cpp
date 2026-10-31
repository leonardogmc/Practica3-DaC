#include <iostream>
#include <vector>
#include <cstdint>
#include <random>
#include <chrono>
#include <iomanip>
#include <string>
#include <utility>

// ============================================================
// QUICKSELECT
// ============================================================

template <typename T>
T quickselect(std::vector<T>& A, int64_t k) {
    k = k - 1;

    int64_t izquierda = 0;
    int64_t derecha = static_cast<int64_t>(A.size()) - 1;

    while (izquierda <= derecha) {
        T pivote = A[(izquierda + derecha) / 2];

        int64_t i = izquierda;
        int64_t j = derecha;

        while (i <= j) {
            while (A[i] < pivote) {
                i += 1;
            }

            while (A[j] > pivote) {
                j -= 1;
            }

            if (i <= j) {
                std::swap(A[i], A[j]);
                i += 1;
                j -= 1;
            }
        }

        if (k <= j) {
            derecha = j;
        } else if (k >= i) {
            izquierda = i;
        } else {
            return A[k];
        }
    }

    return A[izquierda];
}

// ============================================================
// GENERAR DATASET
// ============================================================

template <typename T>
std::vector<T> generar_dataset(size_t n) {
    std::mt19937_64 rng(12345);
    // En C++ el rango de uniform_int_distribution es inclusivo [1, 10000000]
    std::uniform_int_distribution<T> dist(1, 10'000'000);

    std::vector<T> datos(n);
    for (size_t i = 0; i < n; ++i) {
        datos[i] = dist(rng);
    }

    return datos;
}

// ============================================================
// CONVERTIR BYTES A MB
// ============================================================

double bytes_a_mb(size_t bytes) {
    return static_cast<double>(bytes) / (1024.0 * 1024.0);
}

// ============================================================
// MEDIR TIEMPO Y MEMORIA
// ============================================================

template <typename T>
std::pair<double, size_t> ejecutar_quickselect(const std::vector<T>& datos, int64_t k) {
    // Copia del dataset.
    // Quickselect modifica el arreglo durante la ejecución.
    std::vector<T> A = datos;

    // Memoria ocupada por los elementos del arreglo en bytes
    size_t memoria = A.size() * sizeof(T);

    // Medición del tiempo
    auto inicio = std::chrono::high_resolution_clock::now();

    volatile T resultado = quickselect(A, k);
    (void)resultado; // Evita que el compilador elimine la llamada al optimizar

    auto fin = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> duracion = fin - inicio;
    double tiempo = duracion.count();

    return {tiempo, memoria};
}

// ============================================================
// PROGRAMA PRINCIPAL
// ============================================================

int main() {
    std::vector<size_t> tamanos = {
        1000,
        5000,
        10000,
        50000,
        100000,
        500000,
        1000000,
        5000000,
        10000000
    };

    std::cout << std::string(90, '=') << "\n";
    std::cout << "QUICKSELECT - TIEMPO Y CONSUMO DE MEMORIA\n";
    std::cout << std::string(90, '=') << "\n";

    std::cout << std::right
              << std::setw(12) << "n"
              << std::setw(12) << "k"
              << std::setw(18) << "Tiempo 32(s)"
              << std::setw(18) << "Memoria 32(MB)"
              << std::setw(18) << "Tiempo 64(s)"
              << std::setw(18) << "Memoria 64(MB)" << "\n";

    std::cout << std::string(90, '-') << "\n";

    for (size_t n : tamanos) {
        // Posición que queremos encontrar
        int64_t k = static_cast<int64_t>(n / 2);
        if (k < 1) {
            k = 1;
        }

        // ====================================================
        // 32 BITS
        // ====================================================
        auto datos32 = generar_dataset<uint32_t>(n);
        auto [tiempo32, memoria32] = ejecutar_quickselect(datos32, k);

        // ====================================================
        // 64 BITS
        // ====================================================
        auto datos64 = generar_dataset<uint64_t>(n);
        auto [tiempo64, memoria64] = ejecutar_quickselect(datos64, k);

        // ====================================================
        // MOSTRAR RESULTADOS
        // ====================================================
        std::cout << std::right
                  << std::setw(12) << n
                  << std::setw(12) << k
                  << std::fixed << std::setprecision(6) << std::setw(18) << tiempo32
                  << std::fixed << std::setprecision(4) << std::setw(18) << bytes_a_mb(memoria32)
                  << std::fixed << std::setprecision(6) << std::setw(18) << tiempo64
                  << std::fixed << std::setprecision(4) << std::setw(18) << bytes_a_mb(memoria64)
                  << "\n";
    }

    return 0;
}