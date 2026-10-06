# Disciplina de Jogos

[English](#english) · [Português](#português)

---

## English

Projects, exercises and assignments developed for the **Digital Games** course of the undergraduate program at **UTFPR** (Universidade Tecnológica Federal do Paraná).

### Contents

| Module | Description |
| --- | --- |
| `Vector2D` | 2D vector: length, normalization, dot/cross products, arithmetic operators |
| `Transform2D` | 3×3 affine transform (row-major, row vectors): translation, rotation, scale, composition |

### Requirements

- C++17 compiler (GCC or Clang)
- CMake ≥ 3.16
- [GoogleTest](https://github.com/google/googletest), only needed for the tests

### Build and test

From the repository root:

```sh
cmake -S test -B build
cmake --build build
./build/tests
```

To run only some of the tests: `./build/tests --gtest_filter='Vector2D.*'`

### License

[MIT](LICENSE)

---

## Português

Projetos, exercícios e trabalhos desenvolvidos na disciplina de **Jogos Digitais** durante a graduação na **UTFPR** (Universidade Tecnológica Federal do Paraná).

### Conteúdo

| Módulo | Descrição |
| --- | --- |
| `Vector2D` | Vetor 2D: comprimento, normalização, produto escalar/vetorial, operadores aritméticos |
| `Transform2D` | Transformação afim 3×3 (row-major, vetores-linha): translação, rotação, escala, composição |

### Requisitos

- Compilador C++17 (GCC ou Clang)
- CMake ≥ 3.16
- [GoogleTest](https://github.com/google/googletest), necessário apenas para os testes

### Compilar e testar

Na raiz do repositório:

```sh
cmake -S test -B build
cmake --build build
./build/tests
```

Para rodar só uma parte dos testes: `./build/tests --gtest_filter='Vector2D.*'`

### Licença

[MIT](LICENSE)
