
import numpy as np
import time
import tracemalloc


# ==========================================
# MULTIPLICACION TRADICIONAL
# ==========================================

def tradicional(A, B):
    n = A.shape[0]
    C = np.zeros((n, n), dtype=np.int64)

    for i in range(n):
        for j in range(n):
            for k in range(n):
                C[i, j] += A[i, k] * B[k, j]

    return C


# ==========================================
# MULTIPLICACION DE STRASSEN
# ==========================================

def strassen(A, B, limite=16):
    n = A.shape[0]

    # Caso base
    if n <= limite:
        return tradicional(A, B)

    mitad = n // 2

    # Dividir matrices
    A11 = A[:mitad, :mitad]
    A12 = A[:mitad, mitad:]
    A21 = A[mitad:, :mitad]
    A22 = A[mitad:, mitad:]

    B11 = B[:mitad, :mitad]
    B12 = B[:mitad, mitad:]
    B21 = B[mitad:, :mitad]
    B22 = B[mitad:, mitad:]

    # Siete multiplicaciones de Strassen
    M1 = strassen(A11 + A22, B11 + B22, limite)
    M2 = strassen(A21 + A22, B11, limite)
    M3 = strassen(A11, B12 - B22, limite)
    M4 = strassen(A22, B21 - B11, limite)
    M5 = strassen(A11 + A12, B22, limite)
    M6 = strassen(A21 - A11, B11 + B12, limite)
    M7 = strassen(A12 - A22, B21 + B22, limite)

    # Combinar resultados
    C11 = M1 + M4 - M5 + M7
    C12 = M3 + M5
    C21 = M2 + M4
    C22 = M1 - M2 + M3 + M6

    C = np.zeros((n, n), dtype=np.int64)

    C[:mitad, :mitad] = C11
    C[:mitad, mitad:] = C12
    C[mitad:, :mitad] = C21
    C[mitad:, mitad:] = C22

    return C


# ==========================================
# MEDIR TIEMPO Y MEMORIA
# ==========================================

def medir(funcion, A, B, repeticiones=3):
    tiempos = []
    memoria_pico = 0
    resultado = None

    for _ in range(repeticiones):
        tracemalloc.start()

        inicio = time.perf_counter()
        resultado = funcion(A, B)
        fin = time.perf_counter()

        _, pico = tracemalloc.get_traced_memory()
        tracemalloc.stop()

        tiempos.append(fin - inicio)
        memoria_pico = max(memoria_pico, pico)

    tiempo_promedio = sum(tiempos) / len(tiempos)
    memoria_mb = memoria_pico / (1024 ** 2)

    return tiempo_promedio, memoria_mb, resultado


# ==========================================
# PROGRAMA PRINCIPAL
# ==========================================

def main():

    tamanos = [16, 32, 64, 128, 256, 512]

    generador = np.random.default_rng(12345)

    print("\nCOMPARACION DE ALGORITMOS")
    print("=" * 115)

    encabezado = (
        f"{'n':>6} | "
        f"{'Tradicional 2D (s)':>18} | "
        f"{'Mem (MB)':>9} | "
        f"{'Tradicional 1D (s)':>18} | "
        f"{'Mem (MB)':>9} | "
        f"{'Strassen 2D (s)':>16} | "
        f"{'Mem (MB)':>9} | "
        f"{'Strassen 1D (s)':>16} | "
        f"{'Mem (MB)':>9}"
    )

    print(encabezado)
    print("-" * 115)

    for n in tamanos:

        # Generar matrices cuadradas
        A = generador.integers(1, 11, size=(n, n), dtype=np.int64)
        B = generador.integers(1, 11, size=(n, n), dtype=np.int64)

        # Representacion bidimensional
        A2D = A
        B2D = B

        # Representacion unidimensional contigua
        A1D = np.ascontiguousarray(A.ravel())
        B1D = np.ascontiguousarray(B.ravel())

        # Convertir a vistas bidimensionales para usar las mismas
        # operaciones de NumPy sin copiar los datos.
        A1D_2D = A1D.reshape(n, n)
        B1D_2D = B1D.reshape(n, n)

        # Multiplicacion tradicional 2D
        t_trad_2d, m_trad_2d, R1 = medir(
            tradicional, A2D, B2D
        )

        # Multiplicacion tradicional 1D
        t_trad_1d, m_trad_1d, R2 = medir(
            tradicional, A1D_2D, B1D_2D
        )

        # Multiplicacion Strassen 2D
        t_strassen_2d, m_strassen_2d, R3 = medir(
            strassen, A2D, B2D
        )

        # Multiplicacion Strassen 1D
        t_strassen_1d, m_strassen_1d, R4 = medir(
            strassen, A1D_2D, B1D_2D
        )

        # Comprobar que los resultados sean iguales
        if not (
            np.array_equal(R1, R2)
            and np.array_equal(R1, R3)
            and np.array_equal(R1, R4)
        ):
            print(f"Error: resultados diferentes para n={n}")
            return

        # Mostrar resultados
        print(
            f"{n:6} | "
            f"{t_trad_2d:18.6f} | "
            f"{m_trad_2d:9.3f} | "
            f"{t_trad_1d:18.6f} | "
            f"{m_trad_1d:9.3f} | "
            f"{t_strassen_2d:16.6f} | "
            f"{m_strassen_2d:9.3f} | "
            f"{t_strassen_1d:16.6f} | "
            f"{m_strassen_1d:9.3f}"
        )

    print("=" * 115)
    print("Todas las multiplicaciones produjeron resultados iguales.")


if __name__ == "__main__":
    main()
