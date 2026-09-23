# Docrobat

**Docrobat** è un visualizzatore ed editor PDF nativo per **Haiku OS** (x86_64), progettato per integrarsi perfettamente con l'ambiente desktop di Haiku, rispettandone le linee guida di interfaccia utente, la reattività e le convenzioni di sistema.

L'applicazione è sviluppata in **C++20** e sfrutta i kit nativi di Haiku (`AppKit`, `InterfaceKit`, `StorageKit`, `TranslationKit`) insieme alla libreria **`poppler-cpp`** per il parsing e il rendering accurato dei documenti PDF.

---

## Caratteristiche Principali

- **Interfaccia Nativa Haiku:** Costruita con i componenti standard di Haiku (`BApplication`, `BWindow`, `BView`, `BMenuBar`, `BLayoutBuilder`), con supporto completo per temi di sistema e decoratori.
- **Rendering ad Alte Prestazioni:** Conversione diretta delle pagine PDF in Haiku `BBitmap` tramite `poppler-cpp`.
- **Integrazione di Sistema:** Supporto per l'apertura di file dal Tracker via drag & drop o doppio clic (`B_REFS_RECEIVED`).
- **Navigazione Documento:** Scorrimento fluido, salto pagina, zoom regolabile (adattamento a pagina e larghezza) e rotazione.

---

## Requisiti e Dipendenze

Per compilare Docrobat su Haiku OS sono necessari i seguenti pacchetti di sviluppo:

- `gcc` (supporto a C++20)
- `cmake` (versione 3.16 o superiore)
- `make`
- `poppler_devel` (fornisce `libpoppler-cpp`)
- `haiku_devel` (librerie di sistema `libbe`, `libtracker`, `libtranslation`)

Puoi installare le dipendenze richieste tramite `pkgman` da Terminale su Haiku OS:

```bash
pkgman refresh
pkgman install gcc cmake make poppler_devel
```

---

## Compilazione da Sorgente

Per compilare ed eseguire Docrobat su Haiku OS:

```bash
# Clona il repository (se non già clonato)
git clone https://github.com/Brombolo/Docrobat.git
cd Docrobat

# Configura il build con CMake
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release

# Compila il progetto
cmake --build build

# Avvia Docrobat
./build/Docrobat
```

---

## Licenza

Distribuito sotto licenza MIT. Consulta il file `LICENSE` per ulteriori dettagli.
