# 🎮 ALTernetive (v 1.0.0)

> Высокопроизводительный 2D игровой движок на C++ с ECS-архитектурой и встроенным редактором, разработанный в рамках курса ОПД.

<p align="center">
  <img src="https://github.com/user-attachments/assets/a669acfd-5699-426d-a7c4-0e45dc521853"  alt="ALTernetive Banner" />
</p>

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-20-blue.svg" alt="C++20" />
  <img src="https://img.shields.io/badge/SFML-3.0-green.svg" alt="SFML 3.0" />
  <img src="https://img.shields.io/badge/ImGui-Docking-purple.svg" alt="ImGui Docking" />
  <img src="https://img.shields.io/badge/Platform-Windows-blue.svg" alt="Windows" />
  <img src="https://img.shields.io/badge/License-MIT-yellow.svg" alt="MIT License" />
</p>

---

## 📋 Оглавление

- [Возможности](#-возможности)
- [Архитектура](#-архитектура)
- [Требования](#-требования-и-зависимости)
- [Быстрый старт](#-быстрый-старт)
- [Управление](#-управление)
- [Структура проекта](#-структура-проекта)
- [Авторы](#-авторы)

---

## ✨ Возможности

### 🎯 ECS Архитектура
- **Entity Component System** — гибкая и производительная архитектура
- Поддерживаемые компоненты:
  - `Transform` — позиция, поворот, масштаб
  - `Sprite` — отрисовка спрайтов с поддержкой слоёв
  - `Velocity` — скорость и физическое движение
  - `Collider` — Box/Circle коллайдеры с триггерами (в разработке)
  - `Health` — система здоровья (в разработке)
  - `Tag` — теги для идентификации объектов

### 🖥️ Встроенный редактор
- **Viewport** с поддержкой:
  - Перемещение камеры (Middle Mouse Button)
  - Зум (Колёсико мыши)
  - Визуализация границ текстур
  - Drag & Drop перемещение объектов
  
- **Панели редактора**:
  - `Inspector` — просмотр и редактирование компонентов
  - `Content Browser` — навигация по ассетам
  - `Debug Panel` — отладочная информация
  - `Viewport` — окно рендеринга сцены

### 🔧 Инструменты разработчика
- **Контекстное меню** (ПКМ во Viewport):
  - Создание пустых сущностей
  - Мастер создания с пошаговой настройкой
  - Готовые префабы (Player, Enemy, Item, Camera Target)
  
- **Texture Browser** с предпросмотром
- **Система перетаскивания** объектов мышью
- **Горячие клавиши**:
  - `Delete` — удалить выбранную сущность
  - `WASD` / `Стрелки` — управление игроком
  - `Ctrl+Z/Y` — Undo/Redo (в разработке)

### 🎨 Рендеринг
- Поддержка прозрачности и tint-цветов
- Сортировка спрайтов по слоям
- Отладочная отрисовка хитбоксов (возможность отключения в разработке)

---

## 🏗️ Архитектура

```
┌─────────────────────────────────────────────────────────────┐
│                      Application Layer                       │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐  │
│  │   Editor    │  │   Systems   │  │   Resource Manager  │  │
│  │    UI       │  │  Movement   │  │   Texture Cache     │  │
│  │  Panels     │  │   Render    │  │   Font Cache        │  │
│  └─────────────┘  └─────────────┘  └─────────────────────┘  │
├─────────────────────────────────────────────────────────────┤
│                        Core Layer                            │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐  │
│  │   Engine    │  │    Scene    │  │   Entity Manager    │  │
│  │  Game Loop  │◄─┤  Container  │◄─┤   Component Store   │  │
│  └─────────────┘  └─────────────┘  └─────────────────────┘  │
├─────────────────────────────────────────────────────────────┤
│                     Foundation Layer                          │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐  │
│  │   SFML      │  │   ImGui     │  │   Input Manager     │  │
│  │  Window     │  │    UI       │  │   Camera Controller │  │
│  │  Graphics   │  │   Docking   │  │   Entity Dragger    │  │
│  └─────────────┘  └─────────────┘  └─────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

### Модульная структура

| Модуль | Назначение |
|--------|------------|
| **Core** | Основной цикл, инициализация, управление подсистемами |
| **ECS** | Сущности, компоненты, системы (Movement, Render) |
| **Rendering** | Viewport, камера, отладочная отрисовка |
| **Input** | Обработка ввода, камера, перетаскивание объектов |
| **Editor** | ImGui панели, диалоги, инспектор |
| **Resources** | Кэширование текстур, загрузка ассетов |
| **Utils** | Конвертация координат, файловые операции |

---

## 🛠️ Требования и Зависимости

### Для сборки
- **IDE**: Visual Studio 2026 с нагрузкой *"Разработка классических приложений на C++"*
- **CMake**: 3.20+ (в разработке)
- **C++ Standard**: C++20

### Включённые библиотеки (`/libs`)
| Библиотека | Версия | Назначение |
|------------|--------|------------|
| [SFML](https://www.sfml-dev.org/) | 3.0+ | Окна, графика, ввод, аудио |
| [Dear ImGui](https://github.com/ocornut/imgui) | 1.90+ | Интерфейс редактора |
| [imgui-sfml](https://github.com/SFML/imgui-sfml) | 2.6+ | Интеграция ImGui с SFML |

---

## 🚀 Быстрый старт

### Клонирование и сборка

```bash
# Клонируйте репозиторий
git clone https://github.com/Noisy-J/LiteGameEngine.git
cd LiteGameEngine

# Откройте решение в Visual Studio
start LiteGameEngine.slnx

# Нажмите F5 для сборки и запуска
```

### Структура папок для ресурсов
```
LiteGameEngine/
├── assets/
│   ├── textures/
│   │   ├── hero.png          # Спрайт игрока
│   │   └── tusur_logo.png    # Логотип
│   ├── fonts/
│   └── sounds/
├── src/                       # Исходный код
├── libs/                      # Библиотеки
└── LiteGameEngine.slnx        # Решение VS
```

---

## 🎮 Управление

### В редакторе
| Действие | Клавиши |
|----------|---------|
| Перемещение камеры | `СКМ` + Drag |
| Зум | `Колёсико мыши` |
| Выделить объект | `ПКМ` по объекту |
| Переместить объект | `ПКМ` + Drag |
| Контекстное меню | `ПКМ` по пустому месту |
| Удалить объект | `Delete` |
| Инспектор | Панель справа |

### В игре
| Действие | Клавиши |
|----------|---------|
| Движение влево | `A` / `←` |
| Движение вправо | `D` / `→` |
| Движение вверх | `W` / `↑` |
| Движение вниз | `S` / `↓` |

---

## 📁 Структура проекта

```
src/
├── Core/                              # Ядро движка
│   ├── Engine.cpp/hpp                 # Главный класс
│   └── Application.cpp/hpp            # Управление приложением
│
├── ECS/                               # Entity Component System
│   ├── Entity.hpp                     # Тип сущности
│   ├── Scene.cpp/hpp                  # Контейнер сцены
│   ├── Components.hpp                 # Все компоненты
│   └── Systems/
│       ├── MovementSystem.cpp/hpp     # Система движения
│       └── RenderSystem.cpp/hpp       # Система рендеринга
│
├── Rendering/                         # Графическая подсистема
│   ├── Viewport.cpp/hpp               # Вьюпорт и камера
│   └── DebugRenderer.cpp/hpp          # Отладка
│
├── Input/                             # Система ввода
│   ├── InputManager.cpp/hpp           # Менеджер ввода
│   ├── CameraController.cpp/hpp       # Управление камерой
│   └── EntityDragger.cpp/hpp          # Drag & Drop
│
├── Editor/                            # Редактор
│   ├── EditorUI.cpp/hpp               # Главный UI
│   ├── Panels/
│   │   ├── InspectorPanel.cpp/hpp     # Инспектор
│   │   ├── DebugPanel.cpp/hpp         # Отладка
│   │   ├── ViewportPanel.cpp/hpp      # Панель вьюпорта
│   │   └── ContentBrowserPanel.cpp/hpp # Браузер ассетов
│   └── Dialogs/
│       ├── CreateEntityDialog.cpp/hpp # Мастер создания
│       └── TextureSelectorDialog.cpp/hpp # Выбор текстуры
│
├── Resources/                         # Ресурсы
│   ├── ResourceManager.cpp/hpp        # Кэш ресурсов
│   └── TextureBrowser.cpp/hpp         # Сканер текстур
│
├── Utils/                             # Утилиты
│   ├── CoordinateConverter.cpp/hpp    # Конвертация координат
│   └── FileUtils.cpp/hpp              # Работа с файлами
│
└── main.cpp                           # Точка входа
```

---

## 🔮 Roadmap

- [x] ECS архитектура
- [x] Встроенный редактор
- [x] Система компонентов
- [x] Drag & Drop объектов
- [ ] Контекстное меню
- [ ] Мастер создания сущностей
- [ ] Сохранение/загрузка сцен (JSON)
- [ ] Undo/Redo система
- [ ] Физический движок (Box2D)
- [ ] Скриптовая система (Lua)
- [ ] Система частиц
- [ ] Анимация спрайтов
- [ ] Звуковая система
- [ ] Экспорт в исполняемый файл

---

## 🤝 Участие в разработке

Мы приветствуем вклад в развитие движка!

1. Сделайте форк репозитория
2. Создайте ветку для фичи (`git checkout -b feature/amazing-feature`)
3. Зафиксируйте изменения (`git commit -m 'Add amazing feature'`)
4. Отправьте изменения (`git push origin feature/amazing-feature`)
5. Откройте Pull Request

---

## ✒ Авторы

<table>
  <tr>
    <td align="center">
      <a href="https://github.com/Noisy-J">
        <img src="https://github.com/Noisy-J.png" width="100px;" alt="Noisy-J"/>
        <br />
        <sub><b>Noisy-J</b></sub>
      </a>
      <br />
      <sub>Архитектура, ECS, Редактор</sub>
    </td>
    <td align="center">
      <a href="https://github.com/egor417">
        <img src="https://github.com/egor417.png" width="100px;" alt="egor417"/>
        <br />
        <sub><b>egor417</b></sub>
      </a>
      <br />
      <sub>Физика, игровые процессы</sub>
    </td>
  </tr>
</table>

---

## 📄 Лицензия

Проект распространяется под лицензией MIT. Подробнее в файле [LICENSE](LICENSE).

---

<p align="center">
  <b>ALTernetive</b> — создан с ❤️ для курса ОПД
</p>

Этот README содержит всю необходимую информацию: описание возможностей, архитектуру, инструкции по сборке, управление, структуру проекта и roadmap.
