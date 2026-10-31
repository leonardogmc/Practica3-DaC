#include <iostream>
#include <vector>
#include <cstdint>
#include <random>
#include <chrono>
#include <iomanip>
#include <string>
#include <algorithm>
#include <tuple>

// ==========================================
// RASTREADOR DE MEMORIA (EQUIVALENTE A TRACEMALLOC)
// ==========================================

struct RastreadorMemoria {
    inline static size_t actual = 0;
    inline static size_t pico = 0;

    static void reiniciar() {
        actual = 0;
        pico = 0;
    }

    static void reservar(size_t bytes) {
        actual += bytes;
        if (actual > pico) {
            pico = actual;
        }
    }

    static void liberar(size_t bytes) {
        if (actual >= bytes) {
            actual -= bytes;
        } else {
            actual = 0;
        }
    }
};

class RegistroMemoria {
    size_t bytes_ = 0;
public:
    RegistroMemoria() = default;

    explicit RegistroMemoria(size_t bytes) : bytes_(bytes) {
        RastreadorMemoria::reservar(bytes_);
    }

    RegistroMemoria(const RegistroMemoria& otro) : bytes_(otro.bytes_) {
        RastreadorMemoria::reservar(bytes_);
    }

    RegistroMemoria(RegistroMemoria&& otro) noexcept : bytes_(otro.bytes_) {
        otro.bytes_ = 0;
    }

    RegistroMemoria& operator=(const RegistroMemoria& otro) {
        if (this != &otro) {
            RastreadorMemoria::liberar(bytes_);
            bytes_ = otro.bytes_;
            RastreadorMemoria::reservar(bytes_);
        }
        return *this;
    }

    RegistroMemoria& operator=(RegistroMemoria&& otro) noexcept {
        if (this != &otro) {
            RastreadorMemoria::liberar(bytes_);
            bytes_ = otro.bytes_;
            otro.bytes_ = 0;
        }
        return *this;
    }

    ~RegistroMemoria() {
        RastreadorMemoria::liberar(bytes_);
    }
};

// ==========================================
// REPRESENTACION 2D (VECTORES ANIDADOS)
// ==========================================

struct Matriz2D {
    size_t n = 0;
    std::vector<std::vector<int64_t>> datos;
    RegistroMemoria mem;

    Matriz2D() = default;

    explicit Matriz2D(size_t tam)
        : n(tam),
          datos(tam, std::vector<int64_t>(tam, 0)),
          mem(tam * sizeof(std::vector<int64_t>) + tam * tam * sizeof(int64_t)) {}

    size_t tamano() const { return n; }

    int64_t& operator()(size_t i, size_t j) { return datos[i][j]; }
    int64_t operator()(size_t i, size_t j) const { return datos[i][j]; }
};

// ==========================================
// REPRESENTACION 1D (MEMORIA CONTIGUA)
// ==========================================

struct Matriz1D {
    size_t n = 0;
    std::vector<int64_t> datos;
    RegistroMemoria mem;

    Matriz1D() = default;

    explicit Matriz1D(size_t tam)
        : n(tam),
          datos(tam * tam, 0),
          mem(tam * tam * sizeof(int64_t)) {}

    size_t tamano() const { return n; }

    int64_t& operator()(size_t i, size_t j) { return datos[i * n + j]; }
    int64_t operator()(size_t i, size_t j) const { return datos[i * n + j]; }
};

// ==========================================
// OPERACIONES AUXILIARES DE MATRICES
// ==========================================

template <typename Matriz>
Matriz sumar(const Matriz& A, const Matriz& B) {
    size_t n = A.tamano();
    Matriz C(n);
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            C(i, j) = A(i, j) + B(i, j);
        }
    }
    return C;
}

template <typename Matriz>
Matriz restar(const Matriz& A, const Matriz& B) {
    size_t n = A.tamano();
    Matriz C(n);
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            C(i, j) = A(i, j) - B(i, j);
        }
    }
    return C;
}

template <typename Matriz>
Matriz extraer_submatriz(const Matriz& A, size_t fila_ini, size_t col_ini, size_t mitad) {
    Matriz sub(mitad);
    for (size_t i = 0; i < mitad; ++i) {
        for (size_t j = 0; j < mitad; ++j) {
            sub(i, j) = A(fila_ini + i, col_ini + j);
        }
    }
    return sub;
}

template <typename Matriz>
void insertar_submatriz(Matriz& C, const Matriz& sub, size_t fila_ini, size_t col_ini) {
    size_t mitad = sub.tamano();
    for (size_t i = 0; i < mitad; ++i) {
        for (size_t j = 0; j < mitad; ++j) {
            C(fila_ini + i, col_ini + j) = sub(i, j);
        }
    }
}

template <typename MatrizA, typename MatrizB>
bool matrices_iguales(const MatrizA& A, const MatrizB& B) {
    if (A.tamano() != B.tamano()) return false;
    size_t n = A.tamano();
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            if (A(i, j) != B(i, j)) return false;
        }
    }
    return true;
}

// ==========================================
// MULTIPLICACION TRADICIONAL
// ==========================================

template <typename Matriz>
Matriz tradicional(const Matriz& A, const Matriz& B) {
    size_t n = A.tamano();
    Matriz C(n);

    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            int64_t suma = 0;
            for (size_t k = 0; k < n; ++k) {
                suma += A(i, k) * B(k, j);
            }
            C(i, j) = suma;
        }
    }

    return C;
}

// ==========================================
// MULTIPLICACION DE STRASSEN
// ==========================================

template <typename Matriz>
Matriz strassen(const Matriz& A, const Matriz& B, size_t limite = 16) {
    size_t n = A.tamano();

    // Caso base
    if (n <= limite) {
        return tradicional(A, B);
    }

    size_t mitad = n / 2;

    // Dividir matrices
    Matriz A11 = extraer_submatriz(A, 0, 0, mitad);
    Matriz A12 = extraer_submatriz(A, 0, mitad, mitad);
    Matriz A21 = extraer_submatriz(A, mitad, 0, mitad);
    Matriz A22 = extraer_submatriz(A, mitad, mitad, mitad);

    Matriz B11 = extraer_submatriz(B, 0, 0, mitad);
    Matriz B12 = extraer_submatriz(B, 0, mitad, mitad);
    Matriz B21 = extraer_submatriz(B, mitad, 0, mitad);
    Matriz B22 = extraer_submatriz(B, mitad, mitad, mitad);

    // Siete multiplicaciones de Strassen
    Matriz M1 = strassen(sumar(A11, A22), sumar(B11, B22), limite);
    Matriz M2 = strassen(sumar(A21, A22), B11, limite);
    Matriz M3 = strassen(A11, restar(B12, B22), limite);
    Matriz M4 = strassen(A22, restar(B21, B11), limite);
    Matriz M5 = strassen(sumar(A11, A12), B22, limite);
    Matriz M6 = strassen(restar(A21, A11), sumar(B11, B12), limite);
    Matriz M7 = strassen(restar(A12, A22), sumar(B21, B22), limite);

    // Combinar resultados
    Matriz C11 = sumar(restar(sumar(M1, M4), M5), M7);
    Matriz C12 = sumar(M3, M5);
    Matriz C21 = sumar(M2, M4);
    Matriz C22 = sumar(sumar(restar(M1, M2), M3), M6);

    Matriz C(n);
    insertar_submatriz(C, C11, 0, 0);
    insertar_submatriz(C, C12, 0, mitad);
    insertar_submatriz(C, C21, mitad, 0);
    insertar_submatriz(C, C22, mitad, mitad);

    return C;
}

// ==========================================
// MEDIR TIEMPO Y MEMORIA
// ==========================================

template <typename Funcion, typename Matriz>
std::tuple<double, double, Matriz> medir(Funcion funcion, const Matriz& A, const Matriz& B, int repeticiones = 3) {
    double suma_tiempos = 0.0;
    size_t memoria_pico = 0;
    Matriz resultado;

    for (int r = 0; r < repeticiones; ++r) {
        RastreadorMemoria::reiniciar();

        auto inicio = std::chrono::high_resolution_clock::now();
        Matriz temp = funcion(A, B);
        auto fin = std::chrono::high_resolution_clock::now();

        size_t pico_iteracion = RastreadorMemoria::pico;
        memoria_pico = std::max(memoria_pico, pico_iteracion);

        std::chrono::duration<double> duracion = fin - inicio;
        suma_tiempos += duracion.count();

        if (r == repeticiones - 1) {
            resultado = std::move(temp);
        }
    }

    double tiempo_promedio = suma_tiempos / repeticiones;
    double memoria_mb = static_cast<double>(memoria_pico) / (1024.0 * 1024.0);

    return {tiempo_promedio, memoria_mb, resultado};
}

// ==========================================
// PROGRAMA PRINCIPAL
// ==========================================

int main() {
    std::vector<size_t> tamanos = {16, 32, 64, 128, 256, 512};

    std::mt19937_64 generador(12345);
    std::uniform_int_distribution<int64_t> distribucion(1, 10);

    std::cout << "\nCOMPARACION DE ALGORITMOS\n";
    std::cout << std::string(115, '=') << "\n";

    std::cout << std::right
              << std::setw(6) << "n" << " | "
              << std::setw(18) << "Tradicional 2D (s)" << " | "
              << std::setw(9) << "Mem (MB)" << " | "
              << std::setw(18) << "Tradicional 1D (s)" << " | "
              << std::setw(9) << "Mem (MB)" << " | "
              << std::setw(16) << "Strassen 2D (s)" << " | "
              << std::setw(9) << "Mem (MB)" << " | "
              << std::setw(16) << "Strassen 1D (s)" << " | "
              << std::setw(9) << "Mem (MB)" << "\n";

    std::cout << std::string(115, '-') << "\n";

    for (size_t n : tamanos) {
        // Generar matrices en representacion 2D y 1D contigua
        Matriz2D A2D(n), B2D(n);
        Matriz1D A1D(n), B1D(n);

        for (size_t i = 0; i < n; ++i) {
            for (size_t j = 0; j < n; ++j) {
                int64_t valA = distribucion(generador);
                A2D(i, j) = valA;
                A1D(i, j) = valA;
            }
        }

        for (size_t i = 0; i < n; ++i) {
            for (size_t j = 0; j < n; ++j) {
                int64_t valB = distribucion(generador);
                B2D(i, j) = valB;
                B1D(i, j) = valB;
            }
        }

        // Envolturas lambda para invocar las funciones plantilla
        auto fn_trad_2d = [](const Matriz2D& X, const Matriz2D& Y) { return tradicional(X, Y); };
        auto fn_trad_1d = [](const Matriz1D& X, const Matriz1D& Y) { return tradicional(X, Y); };
        auto fn_strassen_2d = [](const Matriz2D& X, const Matriz2D& Y) { return strassen(X, Y, 16); };
        auto fn_strassen_1d = [](const Matriz1D& X, const Matriz1D& Y) { return strassen(X, Y, 16); };

        // Multiplicacion tradicional 2D
        auto [t_trad_2d, m_trad_2d, R1] = medir(fn_trad_2d, A2D, B2D);

        // Multiplicacion tradicional 1D
        auto [t_trad_1d, m_trad_1d, R2] = medir(fn_trad_1d, A1D, B1D);

        // Multiplicacion Strassen 2D
        auto [t_strassen_2d, m_strassen_2d, R3] = medir(fn_strassen_2d, A2D, B2D);

        // Multiplicacion Strassen 1D
        auto [t_strassen_1d, m_strassen_1d, R4] = medir(fn_strassen_1d, A1D, B1D);

        // Comprobar que los resultados sean iguales
        if (!matrices_iguales(R1, R2) || !matrices_iguales(R1, R3) || !matrices_iguales(R1, R4)) {
            std::cout << "Error: resultados diferentes para n=" << n << "\n";
            return 1;
        }

        // Mostrar resultados
        std::cout << std::right
                  << std::setw(6) << n << " | "
                  << std::fixed << std::setprecision(6) << std::setw(18) << t_trad_2d << " | "
                  << std::fixed << std::setprecision(3) << std::setw(9) << m_trad_2d << " | "
                  << std::fixed << std::setprecision(6) << std::setw(18) << t_trad_1d << " | "
                  << std::fixed << std::setprecision(3) << std::setw(9) << m_trad_1d << " | "
                  << std::fixed << std::setprecision(6) << std::setw(16) << t_strassen_2d << " | "
                  << std::fixed << std::setprecision(3) << std::setw(9) << m_strassen_2d << " | "
                  << std::fixed << std::setprecision(6) << std::setw(16) << t_strassen_1d << " | "
                  << std::fixed << std::setprecision(3) << std::setw(9) << m_strassen_1d << "\n";
    }

    std::cout << std::string(115, '=') << "\n";
    std::cout << "Todas las multiplicaciones produjeron resultados iguales.\n";

    return 0;
}