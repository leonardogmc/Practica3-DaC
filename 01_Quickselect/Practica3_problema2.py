import numpy as np
import time


# ============================================================
# QUICKSELECT
# ============================================================

def quickselect(A, k):

    k = k - 1

    izquierda = 0
    derecha = len(A) - 1

    while izquierda <= derecha:

        pivote = A[(izquierda + derecha) // 2]

        i = izquierda
        j = derecha

        while i <= j:

            while A[i] < pivote:
                i += 1

            while A[j] > pivote:
                j -= 1

            if i <= j:

                A[i], A[j] = A[j], A[i]

                i += 1
                j -= 1

        if k <= j:
            derecha = j

        elif k >= i:
            izquierda = i

        else:
            return A[k]

    return A[izquierda]


# ============================================================
# GENERAR DATASET
# ============================================================

def generar_dataset(n, tipo):

    rng = np.random.default_rng(12345)

    return rng.integers(
        1,
        10_000_001,
        size=n,
        dtype=tipo
    )


# ============================================================
# CONVERTIR BYTES A MB
# ============================================================

def bytes_a_mb(bytes):

    return bytes / (1024 ** 2)


# ============================================================
# MEDIR TIEMPO Y MEMORIA
# ============================================================

def ejecutar_quickselect(datos, k):

    # Copia del dataset.
    # Quickselect modifica el arreglo durante la ejecución.
    A = datos.copy()

    # Memoria ocupada por el arreglo
    memoria = A.nbytes

    # Medición del tiempo
    inicio = time.perf_counter()

    quickselect(A, k)

    fin = time.perf_counter()

    tiempo = fin - inicio

    return tiempo, memoria


# ============================================================
# PROGRAMA PRINCIPAL
# ============================================================

def main():

    tamaños = [
        1000,
        5000,
        10000,
        50000,
        100000,
        500000,
        1000000,
        5000000,
        10000000
    ]


    print("=" * 90)
    print("QUICKSELECT - TIEMPO Y CONSUMO DE MEMORIA")
    print("=" * 90)

    print(
        f"{'n':>12}"
        f"{'k':>12}"
        f"{'Tiempo 32(s)':>18}"
        f"{'Memoria 32(MB)':>18}"
        f"{'Tiempo 64(s)':>18}"
        f"{'Memoria 64(MB)':>18}"
    )

    print("-" * 90)


    for n in tamaños:

        # Posición que queremos encontrar
        k = n // 2

        if k < 1:
            k = 1


        # ====================================================
        # 32 BITS
        # ====================================================

        datos32 = generar_dataset(
            n,
            np.uint32
        )

        tiempo32, memoria32 = ejecutar_quickselect(
            datos32,
            k
        )


        # ====================================================
        # 64 BITS
        # ====================================================

        datos64 = generar_dataset(
            n,
            np.uint64
        )

        tiempo64, memoria64 = ejecutar_quickselect(
            datos64,
            k
        )


        # ====================================================
        # MOSTRAR RESULTADOS
        # ====================================================

        print(
            f"{n:>12}"
            f"{k:>12}"
            f"{tiempo32:>18.6f}"
            f"{bytes_a_mb(memoria32):>18.4f}"
            f"{tiempo64:>18.6f}"
            f"{bytes_a_mb(memoria64):>18.4f}"
        )


if __name__ == "__main__":
    main()