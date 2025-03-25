# Instalación de dependencias para el Philosophers Visualizer

Si prefieres instalar manualmente las dependencias necesarias en tu sistema, aquí tienes los comandos para diferentes sistemas operativos:

## Ubuntu/Debian

```bash
# Para tkinter
sudo apt-get update
sudo apt-get install python3-tk

# Para matplotlib y otras dependencias de Python
sudo apt-get install python3-pip
pip3 install matplotlib
```

## Arch Linux

```bash
# Para tkinter
sudo pacman -S tk

# Para matplotlib y otras dependencias de Python
sudo pacman -S python-pip
pip install matplotlib
```

## Fedora

```bash
# Para tkinter
sudo dnf install python3-tkinter

# Para matplotlib y otras dependencias de Python
sudo dnf install python3-pip
pip3 install matplotlib
```

## macOS

```bash
# Usando Homebrew
brew install python-tk
pip3 install matplotlib
```

## Windows

En Windows, tk suele venir incluido en la instalación estándar de Python desde python.org. Solo necesitas instalar matplotlib:

```
pip install matplotlib
```

## Usando un entorno virtual (recomendado)

Si prefieres no instalar paquetes globalmente, puedes usar un entorno virtual:

```bash
# Crear entorno virtual
python3 -m venv venv

# Activar el entorno virtual
# En Linux/macOS:
source venv/bin/activate
# En Windows:
# venv\Scripts\activate

# Instalar dependencias
pip install matplotlib
```

Después de activar el entorno virtual, puedes ejecutar el visualizador dentro de este entorno.
