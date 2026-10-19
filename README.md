# Paired Animation Improvements

**Versión actual:** 1.0.3
**Autor:** [Ersh](https://www.nexusmods.com/skyrimspecialedition/users/39878235) (Ershin en Nexus)
**Juego:** The Elder Scrolls V: Skyrim Special Edition (SE / AE / VR)
**Tipo:** Plugin SKSE (DLL de 64 bits)
**Licencia:** GPL-3.0-or-later con excepciones (ver [Licencia](#licencia))
**Página en Nexus:** [nexusmods.com/skyrimspecialedition/mods/99621](https://www.nexusmods.com/skyrimspecialedition/mods/99621)
**Código fuente:** [github.com/ersh1/PairedAnimationImprovements](https://github.com/ersh1/PairedAnimationImprovements)

---

## Descripción

Paired Animation Improvements es un plugin de SKSE que soluciona los problemas del motor con las **animaciones emparejadas** (*paired animations* / *synchronized clips*), como los remates (*killmoves*), montar/desmontar criaturas, animaciones de travesía emparejadas, etc.

En el juego base, los eventos de animación escritos como **anotaciones** dentro de archivos de animación emparejada **no funcionan**: el motor los ignora. Esto impedía que animadores lanzaran eventos (por ejemplo, sonidos, efectos de partículas, eventos de Payload Interpreter) en animaciones de dos actores. El plugin además libera cientos de ranuras de animación que el motor desperdicia al cargar, algo muy valioso para *replacers* de animaciones.

En esencia, el mod **no añade nada nuevo**: simplemente habilita cosas que las animaciones ya podían hacer pero el motor no permitía.

> *"It's okay if you don't have any idea what that means :) The plugin is just going to enable animators to do some things they couldn't before."* — Ersh

## Características

- ✅ Los **eventos incluidos como anotaciones en animaciones emparejadas** ahora son reconocidos por el juego, igual que en animaciones normales.
- ✅ El motor ya **no pre-crea los bindings de clips sincronizados** al cargar los grafos de comportamiento (*behavior graphs*), lo que **libera unas cuantas ranuras de animación** para ser usadas por *replacers* de animaciones.
- ✅ Compatible con **Skyrim SE (1.5.97), AE (1.6.x / 1.7.99+) y Skyrim VR** (compilación de un solo DLL con offsets dinámicos por variante del runtime).

## Cómo funciona (detalles técnicos)

El motor de hecho **sí leía las anotaciones** de las animaciones emparejadas, pero esperaba un **prefijo** que indicaba qué actor debía disparar el evento:

- `NPC` → actor principal (el jugador o el NPC activador)
- `2_` → el segundo actor de la animación emparejada

El problema: el motor exigía que el **evento con el prefijo también existiera en el grafo de comportamiento** (por ejemplo, `NPCPIE` o `2_PIE`), aunque internamente acababa convirtiéndolo al evento sin prefijo (`PIE`). Esto hacía imposible usar eventos que no existen en el comportamiento, como el evento ficticio *PIE* de [Payload Interpreter](https://www.nexusmods.com/skyrimspecialedition/mods/103516).

El plugin elimina ese requisito **eliminando el prefijo en el momento exacto en que la anotación se lee**, sin necesidad de que el evento prefijado exista en el grafo de comportamiento.

¿Qué hace el código concretamente? (ver [`src/Hooks.cpp`](src/Hooks.cpp) y [`src/Offsets.h`](src/Offsets.h)):

1. **Engancha (hook)** los dos puntos donde se activa el clip interno del `BSSynchronizedClipGenerator`:
   - `BSSynchronizedClipGenerator::Activate` (Address ID `62112` / `63048`, VR: `0xB0CD30`)
   - `BSSynchronizedClipGenerator::Update` (Address ID `62114` / `63050`, VR: `0xB0D0B0`)
2. **Reimplementa** `ConvertAnnotationsToTriggersForSynchronizedClip`: en lugar de dejar pasar únicamente las anotaciones cuyo evento prefijado existe en el grafo, convierte **todas** las anotaciones con el prefijo adecuado en *triggers*, respetando el flag `FLAG_DONT_CONVERT_ANNOTATIONS_TO_TRIGGERS` (0x10) del `hkbClipGenerator`.
3. **Restaura los triggers originales** del clip y añade los provenientes de anotaciones con el tiempo corregido por `cropStartAmountLocalTime`, resolviendo los IDs de evento internos mediante el `hkbSymbolIdMap`.
4. **Corrige los prefijos** de los triggers que no vienen de anotaciones, convirtiendo los eventos `NPCXxx`/`2_Xxx` a sus versiones sin prefijo usando el mapa `eventNamesToIds` del proyecto de animación.
5. **Anula (NOP)** la creación del mapa `eventWithPrefixIdToEventWithoutPrefixIdMap` (ya innecesario) y **hace que `hkbBehaviorGraph::CreateSynchronizedClipBindings` retorne de inmediato** (`xor rax, rax`), evitando la pre-creación de bindings sincronizados.
6. En su lugar, el binding se crea **bajo demanda** con `hkaSkeleton::CreateSynchronizedClipBinding` cuando un clip sincronizado se activa por primera vez (recurso que estaba implementado pero sin usar en el motor vanilla).

Si algo falla durante este proceso (comportamiento nulo, sin eventos, IDs fuera de rango...), el plugin lo registra en su log con nivel `error`/`trace` sin crashear.

## Requisitos (jugadores)

| Requisito | Notas |
| --- | --- |
| [Skyrim Special Edition / Anniversary Edition](https://store.steampowered.com/app/489830) | También compatible con [Skyrim VR](https://store.steampowered.com/app/611670) |
| [Skyrim Script Extender (SKSE64)](https://skse.silverlock.org/) | Obligatorio para cualquier plugin SKSE |
| [Address Library for SKSE Plugins](https://www.nexusmods.com/skyrimspecialedition/mods/32444) | Obligatorio (usa *Address IDs* dinámicos) |

No requiere ninguna dependencia de scripts, ESP ni MCM. Es un único archivo DLL.

## Instalación

1. Instala **SKSE64** y **Address Library for SKSE Plugins** si aún no los tienes.
2. Descarga el archivo principal desde [Nexus](https://www.nexusmods.com/skyrimspecialedition/mods/99621) e instálalo con tu gestor de mods de preferencia (Mod Organizer 2, Vortex, etc.).
3. El plugin se instala en: `Data/SKSE/Plugins/PairedAnimationImprovements.dll`

### Desinstalación

Puedes **instalarlo o desinstalarlo en medio de una partida**: el plugin no deja ningún impacto persistente en la partida (no toca saves, no añade scripts ni objetos). Basta con borrar el DLL.

### Archivos adicionales en Nexus

- **[Opcional] Horse Mount and Dismount Double Sound Fix** (v1.0.2): las animaciones vanilla de montar/desmontar contienen un evento de sonido duplicado que era infuncional sin este plugin. Al instalar PAI, ese evento duplicado ahora sí se dispara y el sonido se reproduce dos veces. Este archivo opcional elimina el evento duplicado de esas animaciones (ediciones de animaciones vanilla realizadas por NickNak).
- **[Misceláneo] Paired Animation Resources by NickNak** (v1.0.0): recursos para animadores — un PDF con instrucciones detalladas de cómo configurar las anotaciones para que el juego las reconozca, animaciones de ejemplo y una lista de los eventos de remate (*killmoves*) vanilla. **Muy recomendable si creas animaciones emparejadas.**

## Compatibilidad

- Compatible con **cualquier versión de Skyrim no anticuada**: 1.5.97 (SE), 1.6.x+ (AE) y VR.
- ✅ Compatible con **Open Animation Replacer v2.0.0 o superior**.
- ❌ **Incompatible con Open Animation Replacer anterior a la v2.0.0** (es necesario actualizar OAR).
- ✅ Compatible con **Dynamic Animation Replacer (DAR)** y con todos los demás mods conocidos.
- No existe versión para Skyrim Legacy Edition (LE) y no se planea (aunque cualquiera puede intentar portarla por su cuenta).

## Registro (log)

En versiones de **Release**, el plugin escribe un log en:

```
Documentos/My Games/Skyrim Special Edition/SKSE/PairedAnimationImprovements.log
```

(En Skyrim VR será: `Documentos/My Games/Skyrim VR/SKSE/PairedAnimationImprovements.log`)

El nivel de log es `info`. Las compilaciones **Debug** escriben trazas detalladas (`trace`) al depurador de Visual Studio (MSVC output) y **esperan a que se conecte un depurador** antes de continuar, por lo que no deben usarse para jugar.

El paquete de la release incluye un archivo **`.pdb`** para obtener mejores registros de crash.

## Compilación (para desarrolladores)

### Requisitos

- [CMake](https://cmake.org/) (añadir a tu `PATH`)
- [Visual Studio Community 2022](https://visualstudio.microsoft.com/) con *Desktop development with C++*
- [Vcpkg](https://github.com/microsoft/vcpkg)
- [The Elder Scrolls V: Skyrim Special Edition](https://store.steampowered.com/app/489830)
- [CommonLibSSE-NG](https://github.com/alandtse/CommonLibSSE-NG) (usado como subdirectorio, no como paquete)

### Variables de entorno requeridas

| Variable | Valor |
| --- | --- |
| `CompiledPluginsPath` | Carpeta donde se copiará el `.dll` compilado (se creará `SKSE/Plugins/` dentro) |
| `VCPKG_ROOT` | Carpeta que contiene tu instalación de vcpkg |
| `CommonLibSSEPath` | Carpeta que contiene tu clon de CommonLibSSE-NG |

### Pasos

```bash
git clone https://github.com/ersh1/PairedAnimationImprovements/
cd PairedAnimationImprovements

cmake --preset vs2022-windows
cmake --build build --config Release
```

Con `COPY_OUTPUT` activado (por defecto), el `.dll` y el `.pdb` se copian automáticamente a `CompiledPluginsPath/SKSE/Plugins/` tras compilar.

### Opciones de CMake

| Opción | Por defecto | Descripción |
| --- | --- | --- |
| `ENABLE_SKYRIM_SE` | `ON` | Soporte de offsets dinámicos para Skyrim SE (1.5.97) |
| `ENABLE_SKYRIM_AE` | `ON` | Soporte de offsets dinámicos para Skyrim AE (1.6.x+) |
| `ENABLE_SKYRIM_VR` | `ON` | Soporte de offsets dinámicos para Skyrim VR |
| `COPY_OUTPUT` | `ON` | Copiar el resultado de la compilación al directorio del juego |

### Detalles de compilación

- Lenguaje: **C++20**
- Generador: Visual Studio 17 2022 (toolset v143, arquitectura x64)
- Triplet vcpkg: `x64-windows-static-md`
- Dependencias vcpkg ([`vcpkg.json`](vcpkg.json)): `directxtk`, `fmt`, `rapidcsv`, `rsm-binary-io`, `spdlog`
- Flags: `/W4 /WX` (advertencias como errores), `/permissive-`, `/Zc:preprocessor`
- Empaquetado: `cpack` genera un **ZIP** en `build/packaging/` con los componentes `main` (DLL) y `pdbs` (PDB)

### Estructura del proyecto

```
├── CMakeLists.txt          # Proyecto raíz (versión 1.0.3, opciones, subdirectorio src/)
├── CMakePresets.json       # Preset vs2022-windows (VS2022 + vcpkg + Windows)
├── vcpkg.json              # Dependencias
├── cmake/
│   ├── Plugin.h.in         # Cabecera generada con nombre y versión del plugin
│   ├── version.rc.in       # Info de versión del recurso Windows
│   └── packaging.cmake     # Configuración de CPack (ZIP)
└── src/
    ├── main.cpp            # Punto de entrada SKSE (Query/Load), inicialización del log
    ├── Hooks.h / Hooks.cpp # Toda la lógica: hooks y parches sobre BSSynchronizedClipGenerator
    ├── Offsets.h           # Relocaciones (Address IDs SE/AE/VR) de funciones de Havok/Behavior
    ├── RE/Havok.h          # Definiciones reverse-engineered de estructuras Havok/Behavior
    ├── PCH.h               # Precompiled headers (CommonLibSSE, spdlog, SKSE)
    └── CMakeLists.txt      # Target SHARED, compilación y post-build copy
```

### Notas de implementación

- Los offsets usan `REL::VariantID(idSE, idAE, offsetVR)`, el mecanismo de offsets dinámicos de CommonLibSSE-NG, por lo que un único DLL sirve para las tres variantes del juego.
- `main.cpp` llama a `REL::Module::reset()` como *workaround* de un bug de CommonLibSSE-NG.
- En `SKSEPlugin_Query` se rechazan cargas en el editor del CK y runtimes anteriores a 1.5.39 (SE) o al último conocido (VR).
- Los hooks de depuración (`LogHooks`, que vuelcan a log todos los triggers pre/post activación) solo se instalan en compilaciones Debug (`#ifndef NDEBUG`).

## Changelog

| Versión | Cambios |
| --- | --- |
| **1.0.3** | Soporte para el runtime 1.7.99+ |
| **1.0.2** | Corregidas algunas animaciones emparejadas rotas (p. ej., montar un dragón) |
| **1.0.1** | Corregido un crash en VR causado por un offset de memoria incorrecto; añadido el archivo `.pdb` para mejores registros de crash |
| **1.0.0** | Primera versión (30 de agosto de 2023). Originalmente llamado *Paired Animation Annotation Fix* |

## Preguntas frecuentes

**¿Puedo instalarlo/desinstalarlo a mitad de partida?**
Sí. No tiene ningún impacto persistente en tu partida.

**¿Es necesario para jugar?**
No, pero si usas mods que añaden animaciones emparejadas con eventos (sonidos, partículas, Payload Interpreter), estos no funcionarán correctamente sin él. Además, las animaciones vanilla de montar/desmontar pueden reproducir el sonido dos veces (usa el archivo opcional *Horse Mount and Dismount Double Sound Fix* para evitarlo).

**¿Existe versión para Skyrim LE?**
No. El motor de la Special Edition es mucho más estable y frameworks como CommonLibSSE facilitan enormemente el desarrollo de plugins avanzados.

**¿Puedo contribuir?**
Sí. El autor invita a enviar *pull requests* a GitHub si quieres añadir funcionalidades o correcciones.

## Enlaces

- [Página del mod en Nexus Mods](https://www.nexusmods.com/skyrimspecialedition/mods/99621)
- [Repositorio en GitHub](https://github.com/ersh1/PairedAnimationImprovements)
- [SKSE (Skyrim Script Extender)](https://skse.silverlock.org/)
- [Address Library for SKSE Plugins](https://www.nexusmods.com/skyrimspecialedition/mods/32444)
- [CommonLibSSE-NG](https://github.com/alandtse/CommonLibSSE-NG)
- [Open Animation Replacer](https://www.nexusmods.com/skyrimspecialedition/mods/92109)
- [Payload Interpreter](https://www.nexusmods.com/skyrimspecialedition/mods/103516)

## Licencia

[GPL-3.0-or-later](COPYING) WITH [Modding Exception AND GPL-3.0 Linking Exception (with Corresponding Source)](EXCEPTIONS).

Específicamente, el *Modded Code* es Skyrim (y sus variantes), y las *Modding Libraries* incluyen [SKSE](https://skse.silverlock.org/) y CommonLib (y sus variantes).
