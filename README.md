# Escape Chronicles

---

## Language - EN

## 🎲 Game Overview

Welcome to the repository of Escape Chronicles — a three-dimensional prison escape simulator inspired by The Escapists 2! This is a dynamic cooperative game developed on Unreal Engine 5 using C++. Players must meticulously plan their escape from a heavily guarded prison, utilizing stealth, crafting, and teamwork. The game supports up to 4 players via P2P connection on Steam.

You are a prisoner who must adhere to the prison schedule, interact with NPCs (inmates, guards, medics), gather resources, craft tools, and find paths to freedom while avoiding suspicion. NPCs are controlled by advanced AI based on State Trees and Smart Objects, ensuring realistic behavior.

## ▶️ Gameplay Video
Watch gameplay: [Link](https://www.youtube.com/watch?v=SWOnRBJJU0c)

## ⚙️ Core Mechanics and Features

### 👨‍🚀 Character Attributes

- Health: When reduced to 0, the character faints, and medics transport them to the infirmary.
- Energy: Consumed by running, attacking, and destroying objects.
- Suspicion: At 90 or above, guards will attack the prisoner.
- Damage: Depends on the weapon used or its absence.

### 🏃‍♂️ Character Movement

- Implemented using Mover and integrated with the Gameplay Ability System (GAS).
- The character can walk, jog, sprint (consuming energy), crouch, and jump.
- Character orientation follows the camera, with animations supporting in-place turns and movement in any direction.

### 🎒 Inventory and Crafting

- The system is based on dividing items into ItemDefinitions and ItemInstances, where each ItemDefinition and the inventory itself have fragments that constitute their logic. This system was inspired by Lyra Starter Game.
- The character has a limited number of inventory slots and a separate slot for clothing.
- The character can select a specific slot to interact with the item in that slot.
- Items can be picked up and dropped.
- Crafting system: Players can combine items to create new ones.
- Item types: Regular or contraband, consumables, tools, weapons, clothing.
- Contraband in the player’s inventory is detected by metal detectors, raising suspicion to the maximum. This can be bypassed if the player has a contraband pouch in their inventory.
- Upon fainting, the player loses all contraband items.
- Some items have durability and can break.
- Players can craft a dummy for the bed to deceive guards during nightly checks.
- Players can search the inventory of fainted characters.
- Players can search cabinets.
- Items in cabinets and bot inventories are updated daily.

### 🛠️ Object Interaction

Characters can interact with various objects on the map.

Main interactive objects:
- Beds, hospital beds, showers, seats: Restore health or energy.
- Items on the map: Can be picked up into the inventory.
- Cabinets: Used for storing items or searching.
- Fainted characters: Can be searched.

### 🔨 Object Destruction

- Players can destroy specific surfaces using appropriate tools.
- Implemented through DynamicMeshActor and Geometry Script.
- Guards raise an alarm upon discovering destruction, but players can act in areas where guards typically do not patrol.
- Destruction unlocks previously inaccessible areas or escape routes.

### 👕 Clothing

- Prisoners, guards, and medics have different clothing.
- Clothing can be stolen or crafted.
- A disguised player does not trigger an alarm when outside their cell at night or in restricted areas.

### ⚔️ Combat System

- Attacks deal damage and consume energy.
- Blocking attacks prevents damage while consuming energy.

### 🚪 Doors and Keys

- Doors can be open, locked, or have one-way access.
- Passing through locked doors requires a key of the corresponding color.
- Keys can be found, stolen, or duplicated.
- Original keys do not disappear when crafting molds for duplicates.
- Key duplicates lose durability with use.
- A guard raises an alarm if they wake up without their key.

### ⏰📅 Time and Event System

Game Time:
- Divided into days, hours, and minutes (1 minute of game time = 1 second of real time).
- Day and night cycles affect map lighting.
- Specific light sources on the map turn on or off at certain times.

Game Events:
- Quiet Hour: Prisoners must return to their cells for a set period. Afterward, cell doors lock for exiting, and many other doors lock completely.
- Roll Call: Prisoners must appear in the common area, or an alarm is triggered.
- Meal Time: Prisoners sit at tables in the dining area.
- Shower: Prisoners use the shower.
- Break: Free time.
- Alarm: Pauses other events (though bot prisoners continue following them). Triggered if a prisoner is absent from their cell at night, misses roll call, damage is detected in the prison, or a guard wakes up without their key. After an alarm, all map damage is repaired. Alarms are raised for specific characters depending on the cause.
- Missing any scheduled event increases a prisoner’s suspicion or triggers an alarm.

### 🤖 NPC Artificial Intelligence

- NPC Types: Prisoners, guards, medics.
- Implemented using State Trees, Smart Objects, AI Controllers, Detour Crowd Avoidance, Navigation Mesh, and NavLinkProxies.
- Behavior depends on the role and current event.
- Guards:
  - Follow the daily schedule.
  - Patrol, monitoring prisoners and checking for prison damage.
  - Attack prisoners with high suspicion levels.
  - Check cells for missing prisoners at night.
- Medics:
  - Respond to fainted characters and transport them to the infirmary.
- Prisoners:
  - Follow the daily schedule.
  - Use beds, showers, and other objects.
- In a passive state, NPCs move within permitted zones, divided for different NPC types using NavArea.

### 🥈 Escape

- The game ends when one player successfully escapes.
- Escape condition: Exit the prison through a destroyed zone while remaining undetected.

### 🖥️ UI

The UI, implemented through Common UI, provides:
- Main Menu: Start game, settings, exit.
- Pause Menu: Resume, settings, return to main menu.
- Settings Menu: Volume, graphics quality.
- Player HUD: Attributes, time, current event, presence marker, inventory.
- Character Menu: Name, clothing, crafting.
- Victory Screen: Displays escape time.
- Player names are visible above their heads (text oriented toward the camera).

Additional game menus include, for example, the open cabinet menu or another character’s inventory.

### 💾 Save System

- Implemented through serialization of fields marked with `UPROPERTY(SaveGame)`.
- Includes:
  - Auto-loading when the host creates the game.
  - Auto-saving every few seconds and at the end of the game.
  - Auto-saving for a specific player upon their exit.
  - Loading for a joining player upon entry.
- Saves:
  - All characters, their attributes, and states.
  - All inventories, cabinets, and items on the map.
  - Dynamic map damage.
  - Active event and its data.
  - Current game time.
- Supports online and offline players, as well as bots.
- Each character has a unique identifier.
- Supports transfer of save files between hosts.

### 🌐 Network Mode

- Implemented through OnlineInterface, OnlineSessions, and Steam Sockets.
- Supports up to 4 players simultaneously.
- The host creates the game and owns the save. Clients connect via Steam invitations.

## 👥 Project Team

## Vladislav Semchuk

*(also known as Driver2579)*

[LinkedIn](https://www.linkedin.com/in/vladislav-semchuk-a2b944203/)

Role: Team Lead, Project Manager, Programmer.

Contributions:
- Designing the overall game architecture.
- Task and deadline planning.
- Role assignment.
- Code quality control.
- Integration of Gameplay Ability System (GAS).
- Character control system via GAS.
- Character movement using Mover and GAS.
- Modular Gameplay Abilities component system.
- Support for different PlayerStates for different controllers for modular GAS setup and beyond for various character types.
- Save system and its support across all project components.
- Character identification.
- Object ownership system via component.
- Game time system.
- Game event system and the events themselves.
- Event presence marker system.
- Day and night cycle system.
- Component for enabling/disabling lights based on current game time.
- Steam integration and P2P connection support.
- Character teleportation to bed upon first game entry.
- Bot spawning and custom PlayerStart with character tag filtering.
- Core NPC and AI bot logic (patrolling, alarms, behavior).
- Artificial Intelligence (AI).
- Contraband items.
- Metal detector.
- Clothing items and character model switching.
- Item usage on a dedicated button.
- Consumable items.
- Bed dummy item.
- Main player HUD UI, excluding inventory (attributes, time, current event, presence marker).
- Main menu UI without design.
- Pause menu UI functionality (opening settings and transitioning to main menu).
- Settings system and UI.
- Character name UI above head.
- Object destructibility component.
- Music manager for game events.
- Music sourcing.
- Miscellaneous assistance to other team members.

### Andrey Shelengovskiy

[LinkedIn](https://www.linkedin.com/in/andrey-shelengovskiy/)

Role: Programmer.

Contributions:
- Inventory system.
- Crafting system.
- Cabinets.
- Character searching.
- UI system.
- Inventory UI in player HUD.
- Character menu UI (name, clothing, crafting).
- Cabinet UI.
- Character search UI.
- Styles for main menu UI.
- Styles for settings menu UI.
- Pause menu UI (open/close functionality and styles).
- All interactive objects (beds, hospital beds, showers, seats, etc.).
- Combat system.
- Tool items and their use for breaking objects via the destructibility component.
- Weapon items.
- Doors and keys.
- Item durability.
- Character pickup.
- Player victory trigger for escape.
- Player victory UI.
- Animations (sourcing and implementation).
- Sounds (sourcing and implementation).

### Yuliya Vrublevskaya

Role: Designer.

Contributions:
- Level design.
- 3D models.
- UI design.
- Music sourcing.
- Sound sourcing.

---

## Мова - UA

## 🎲 Огляд гри

Ласкаво просимо до репозиторію Escape Chronicles — тривимірного симулятора втечі з в’язниці, натхненного The Escapists 2! Це динамічна кооперативна гра, розроблена на Unreal Engine 5 з використанням C++. Гравцям доведеться ретельно планувати втечу з охоронюваної в’язниці, використовуючи стелс, крафт та командну роботу. Підтримується до 4 гравців через P2P-з’єднання в Steam.

Ви — ув'язнений, який має дотримуватись тюремного розпорядку, взаємодіяти з NPC (ув'язнені, охоронці, медики), збирати ресурси, створювати інструменти та шукати шлях до свободи, уникаючи підозр. NPC керуються просунутим ІІ на основі State Trees та Smart Objects, що забезпечує реалістичну поведінку.

## ⚠️ Важливо

Escape Chronicles все ще перебуває в активній розробці.
Поточна версія гри може містити баги та недопрацювання. Візуальна складова, частина геймплею та інші аспекти гри можуть значно змінитися у фінальній версії.
Відео геймплею нижче демонструє одну з проміжних версій і може не відображати поточний стан гри.
Останню актуальну версію завжди можна завантажити на сторінці релізів.

## ▶️ Відео геймплею

Дивитись геймплей: [Посилання](https://youtu.be/SWOnRBJJU0c)

## ⚙️ Основні механіки та можливості

### 👨‍🚀 Характеристики персонажа

- Здоров'я: при падінні до 0 персонаж втрачає свідомість, і медики переносять його до лазарету.
- Енергія: витрачається на біг, удари, руйнування об'єктів.
- Підозрілість: при досягненні 90 і вище охоронці атакують ув'язненого.
- Урон: залежить від використовуваної зброї або її відсутності.

### 🏃‍♂️ Пересування персонажа

- Реалізовано за допомогою Mover та інтегровано з Gameplay Ability System (GAS).
- Персонаж може ходити, бігти підтюпцем, бігти швидко (що витрачає енергію), присідати та стрибати.
- Орієнтація персонажа завжди слідує за камерою, анімації підтримують повороти на місці та рух у будь-якому напрямку.

### 🎒 Інвентар та крафт

- Система заснована на поділі предметів на ItemDefinitions та ItemInstances, де кожен ItemDefinition та сам інвентар мають фрагменти, з яких складається їх логіка. Ця система була натхненна Lyra Starter Game.
- Персонаж має обмежену кількість слотів та окремий слот під одяг;
- Персонаж може виділити певний слот, щоб взаємодіяти з предметом у цьому слоті.
- Предмети можна підбирати та викидати.
- Система крафту: гравець може комбінувати предмети для створення нових.
- Типи предметів: звичайні або контрабандні, витратні, інструменти, зброя, одяг.
- Контрабанда в інвентарі гравця виявляється металодетекторами, підвищуючи підозрілість до максимуму. Однак це можна обійти, якщо у гравця в інвентарі є контрабандний мішок.
- При втраті свідомості гравець втрачає всі контрабандні предмети.
- Деякі предмети мають міцність і можуть ламатися.
- Гравець може створювати ляльку для ліжка, щоб обдурити охоронців під час нічних перевірок.
- Гравець може обшукувати інвентар інших персонажів без свідомості.
- Гравець може обшукувати тумбочки.
- Предмети в тумбочках та інвентарях ботів оновлюються щодня.

### 🛠️ Взаємодія з об'єктами

Персонаж може взаємодіяти з різними об'єктами на карті.

Основні об'єкти для взаємодії:
- Ліжка, лікарняні койки, душові, сидіння: відновлюють здоров'я або енергію.
- Предмети на карті: можна підібрати в інвентар.
- Тумбочки: використовуються для зберігання предметів або обшуку.
- Персонажі без свідомості: доступні для обшуку.

### 🔨 Руйнування об'єктів

- Гравець може руйнувати певні поверхні за допомогою відповідних інструментів.
- Реалізовано через DynamicMeshActor та Geometry Script.
- Охоронці піднімають тривогу при виявленні руйнувань, але можна діяти в зонах, де охоронці зазвичай не ходять.
- Руйнування відкривають раніше недоступні зони або шлях до втечі.

### 👕 Одяг

- Ув'язнені, охоронці та медики мають різний одяг.
- Одяг можна вкрасти або скрафтити.
- Замаскований гравець не викликає тривогу, перебуваючи поза камерою вночі або в заборонених зонах.

### ⚔️ Бойова система

- Атака завдає шкоди та витрачає енергію.
- Блокування ударів не дає отримати шкоду, витрачаючи енергію натомість.

### 🚪 Двері та ключі

- Двері бувають відкритими, зачиненими або з одностороннім доступом.
- Для проходу через зачинені двері потрібен ключ відповідного кольору.
- Ключі можна знайти, вкрасти або створити копію.
- Оригінали ключів не зникають при крафті зліпків для їх копій.
- Копії ключів втрачають міцність при використанні.
- Охоронець піднімає тривогу, якщо прокидається без ключа.

### ⏰📅 Система часу та подій

Ігровий час:

- Поділено на дні, години та хвилини (1 хвилина ігрового часу = 1 секунда реального часу).
- Зміна дня та ночі впливає на освітлення карти.
- У певний час на карті вмикаються або вимикаються певні джерела освітлення.

Ігрові події:

- Тихий час: протягом певного часу ув'язнені мають повернутися до своїх камер. Після цього двері в камери зачиняються на вихід, а багато інших дверей — повністю.
- Перекличка: ув'язнені мають з'явитися у загальному залі, інакше почнеться тривога.
- Прийом їжі: ув'язнені сідають за столи в їдальні.
- Душ: ув'язнені користуються душем.
- Перерва: вільний час.
- Тривога: ставить інші події на паузу (але боти-заложники продовжують їх виконувати). Викликається, якщо ув'язненого немає в камері вночі, пропущена перекличка, у в'язниці виявлені пошкодження або охоронець прокидається без свого ключа. Після тривоги всі пошкодження карти усуваються. Тривога піднімається для конкретного списку персонажів залежно від причини її початку.
- Пропуск ув'язненим будь-якої події розпорядку дня підвищує його підозрілість або викликає тривогу.

### 🤖 Штучний інтелект NPC

- Типи NPC: ув'язнені, охоронці, медики.
- Реалізовано через State Trees, Smart Objects, AI Controllers, Detour Crowd Avoidance, Navigation Mesh та NavLinkProxies.
- Поведінка залежить від ролі та поточної події.
- Охоронці:
  - Слідують розпорядку дня.
  - Патрулюють, слідкують за ув'язненими та шукають пошкодження в'язниці.
  - Атакують ув'язнених з високим рівнем підозрілості.
  - Перевіряють камери на відсутність ув'язнених вночі.
- Медики:
  - Реагують на персонажів без свідомості та несуть їх до лазарету.
- Ув'язнені:
  - Слідують розпорядку дня.
  - Використовують ліжка, душ та інші об'єкти.
- У пасивному стані переміщуються по дозволених зонах, поділених для різних типів NPC завдяки NavArea.

### 🥈 Втеча

- Гра завершується, коли один із гравців успішно тікає.
- Умова втечі: вийти за межі в'язниці через зруйновану зону, залишаючись непоміченим.

### 🖥️ UI

UI, реалізований через Common UI, надає:
- Головне меню: запуск гри, налаштування, вихід.
- Меню паузи: продовжити, налаштування, вихід у головне меню.
- Меню налаштувань: гучність, якість графіки.
- HUD гравця: характеристики, час, поточна подія, відмітка присутності, інвентар.
- Меню персонажа: ім'я, одяг, крафт.
- Екран перемоги: відображає час втечі.
- Імена гравців видно над головами (текст повернутий до камери).

Також є інші різноманітні ігрові меню, наприклад, меню відкритої тумбочки або чужого інвентарю.

### 💾 Система збережень

- Реалізована через серіалізацію полів, позначених як `UPROPERTY(SaveGame)`.
- Включає:
  - Автозавантаження при створенні гри хостом.
  - Автозбереження кожні кілька секунд і при завершенні гри.
  - Автоматичне збереження конкретного гравця при його виході.
  - Завантаження приєднаного гравця при його вході.
- Зберігаються:
  - Всі персонажі, їх атрибути та стани.
  - Всі інвентарі, тумбочки та предмети на карті.
  - Динамічні пошкодження карти.
  - Активна подія та її дані.
  - Поточний ігровий час.
- Підтримуються онлайн- та офлайн-гравці, а також боти.
- Кожен персонаж має унікальний ідентифікатор.
- Підтримується передача файлів збережень між хостами.

### 🌐 Мережевий режим

- Реалізовано через OnlineInterface, OnlineSessions та Steam Sockets.
- Підтримує до 4 гравців одночасно.
- Хост створює гру та володіє збереженням. Клієнт підключається через запрошення в Steam.

## 👥 Команда проєкту

### Владислав Семчук

*(також відомий як Driver2579)*

[LinkedIn](https://www.linkedin.com/in/vladislav-semchuk-a2b944203/)

Роль: Тімлід, проджект-менеджер, програміст.

Внесок:
- Проєктування загальної архітектури гри.
- Планування завдань і термінів.
- Розподіл ролей.
- Контроль якості коду.
- Інтеграція Gameplay Ability System (GAS).
- Система керування персонажем через GAS.
- Пересування персонажа з використанням Mover і GAS.
- Система модульних компонентів Gameplay Abilities.
- Підтримка різних PlayerStates для різних контролерів для модульного налаштування GAS і не тільки для різних типів персонажів.
- Система збережень і її підтримка всіма компонентами проєкту.
- Ідентифікація персонажів.
- Система власників об'єктів через компонент.
- Система ігрового часу.
- Система ігрових подій і самі ігрові події.
- Система відмітки на ігрових подіях.
- Система зміни дня і ночі.
- Компонент для увімкнення/вимкнення світла залежно від поточного ігрового часу.
- Інтеграція Steam і підтримка P2P-підключення.
- Телепортація персонажа в ліжко при першому вході в гру.
- Спавн ботів і кастомний PlayerStart з фільтром за тегами персонажа.
- Створення основної логіки NPC і AI ботів (патрулювання, тривоги, поведінка).
- Штучний інтелект (ШІ).
- Контрабандні предмети.
- Металодетектор.
- Предмети одягу та зміна моделі персонажа.
- Використання предметів на окрему кнопку.
- Витратні предмети.
- Предмет ляльки для ліжка.
- UI основного HUD гравця, окрім інвентаря (характеристики, час, поточна подія, відмітка присутності).
- UI головного меню без дизайну.
- Функціонал UI меню паузи (відкриття меню налаштувань і перехід у головне меню).
- Система та UI меню налаштувань.
- UI імені над головою персонажа.
- Компонент для руйнування об'єктів.
- Менеджер музики для ігрових подій.
- Пошук музики.
- Інша допомога учасникам команди.

### Андрій Шеленгівський

[LinkedIn](https://www.linkedin.com/in/andrey-shelengovskiy/)

Роль: Програміст.

Внесок:
- Система інвентаря.
- Система крафту.
- Тумбочки.
- Огляд персонажів.
- Система для UI.
- UI інвентаря в HUD гравця.
- UI меню персонажа (ім'я, одяг, крафт).
- UI тумбочок.
- UI обшуку персонажів.
- Стилі для UI головного меню.
- Стилі для UI меню налаштувань.
- UI меню паузи (можливість відкрити та закрити, і стилі).
- Всі об'єкти для взаємодії (ліжка, лікарняні койки, душові, сидіння тощо).
- Бойова система.
- Предмети інструментів та їх використання для ламання об'єктів через компонент руйнування.
- Предмети зброї.
- Двері та ключі.
- Міцність предметів.
- Підбір персонажів.
- Тригер перемоги гравців для втечі.
- UI перемоги гравців.
- Анімації (пошук і реалізація).
- Звуки (пошук і реалізація).

### Юлія Врублевська

Роль: Дизайнер.

Внесок:
- Level-дизайн.
- 3D-моделі.
- UI-дизайн.
- Пошук музики.
- Пошук звуків.

---

## Язык - RU

## 🎲 Обзор игры

Добро пожаловать в репозиторий Escape Chronicles — трёхмерного симулятора побега из тюрьмы, вдохновлённого The Escapists 2! Это динамичная кооперативная игра, разработанная на Unreal Engine 5 с использованием C++. Игрокам предстоит тщательно планировать побег из охраняемой тюрьмы, применяя стелс, крафт и командную работу. Поддерживается до 4 игроков через P2P-соединение в Steam.

Вы — заключённый, который должен соблюдать тюремный распорядок, взаимодействовать с NPC (заключённые, охранники, медики), собирать ресурсы, создавать инструменты и искать пути к свободе, избегая подозрений. NPC управляются продвинутым ИИ на основе State Trees и Smart Objects, что обеспечивает реалистичное поведение.

## ⚠️ Важно

Escape Chronicles всё ещё находится в активной разработке.
Текущая версия игры может содержать баги и недоработки. Визуальная составляющая, некоторая часть геймплея и другие аспекты игры могут значительно измениться в финальной версии.
Видео геймплея ниже демонстрирует одну из промежуточных версий и может не отражать текущее состояние игры.
Последнюю актуальную версию всегда можно скачать на странице релизов.

## ▶️ Видео геймплея
Смотреть геймплей: [Ссылка](https://youtu.be/SWOnRBJJU0c)

## ⚙️ Основные механики и возможности

### 👨‍🚀 Характеристики персонажа

- Здоровье: при падении до 0 персонаж теряет сознание, и медики переносят его в лазарет.
- Энергия: расходуется на бег, удары, разрушение объектов.
- Подозрительность: при достижении 90 и выше охранники атакуют заключённого.
- Урон: зависит от используемого оружия или его отсутствия.

### 🏃‍♂️ Передвижение персонажа

- Реализовано с помощью Mover и интегрировано с Gameplay Ability System (GAS).
- Персонаж может ходить, бегать трусцой, бегать быстро (что тратит энергию), приседать и прыгать.
- Ориентация персонажа всегда следует за камерой, анимации поддерживают повороты на месте и движение в любом направлении.

### 🎒 Инвентарь и крафт

- Система основана на делении предметов на ItemDefinitions и ItemInstances, где каждый ItemDefinition и сам инвентарь имеют фрагменты, из которых состоит их логика. Данная система была вдохновлена Lyra Starter Game.
- Персонаж имеет ограниченное число слотов и отдельный слот под одежду;
- Персонаж может выделить определённый слот, чтобы взаимодействовать с предметом в этом слоте.
- Предметы можно подбирать и выбрасывать.
- Система крафта: игрок может комбинировать предметы для создания новых.
- Типы предметов: обычные или контрабандные, расходуемые, инструменты, оружие, одежда.
- Контрабанда в инвентаре игрока обнаруживается металлодетекторами, повышая подозрительность до максимума. Однако это можно обойти, если у игрока в инвентаре есть контрабандный мешок.
- При потере сознания игрок теряет все контрабандные предметы.
- Некоторые предметы имеют прочность и могут ломаться.
- Игрок может создавать куклу для кровати, чтобы обмануть охранников во время ночных проверок.
- Игрок может обыскивать инвентарь других персонажей без сознания.
- Игрок может обыскивать тумбочки.
- Предметы в тумбочках и инвентарях ботов обновляются ежедневно.

### 🛠️ Взаимодействие с объектами

Персонаж может взаимодействовать с различными объектами на карте.

Основные объекты для взаимодействия:
- Кровати, больничные койки, душевые, сиденья: восстанавливают здоровье или энергию.
- Предметы на карте: можно подобрать в инвентарь.
- Тумбочки: используются для хранения предметов или обыска.
- Персонажи без сознания: доступны для обыска.

### 🔨 Разрушение объектов

- Игрок может разрушать определённые поверхности с помощью подходящих инструментов.
- Реализовано через DynamicMeshActor и Geometry Script.
- Охранники поднимают тревогу при обнаружении разрушений, но можно действовать в зонах, где охранники обычно не ходят.
- Разрушения открывают ранее недоступные зоны или путь к побегу.

### 👕 Одежда

- Заключённые, охранники и медики имеют разную одежду.
- Одежду можно украсть или скрафтить.
- Замаскированный игрок не вызывает тревогу, находясь вне камеры ночью или в запретных зонах.

### ⚔️ Боевая система

- Атака наносит урон и тратит энергию.
- Блокировка ударов не даёт получить урон, расходуя энергию взамен.

### 🚪 Двери и ключи

- Двери бывают открытыми, закрытыми или с односторонним доступом.
- Для прохода через закрытые двери нужен ключ соответствующего цвета.
- Ключи можно найти, украсть или создать копию.
- Оригиналы ключей не пропадают при крафте слепков для их копий.
- Копии ключей теряют прочность при использовании.
- Охранник поднимает тревогу, если просыпается без ключа.

### ⏰📅 Система времени и событий

Игровое время:
- Поделено на дни, часы и минуты (1 минута игрового времени = 1 секунда реального времени).
- Смена дня и ночи влияет на освещение карты.
- В определённое время на карте включаются или выключаются определённые источники освещения.

Игровые события:
- Тихий час: в течение определённого времени заключённые должны вернуться в свои камеры. После этого двери в камеры закрываются на выход, а многие другие двери — полностью.
- Перекличка: заключённые должны появиться в общем зале, иначе начнётся тревога.
- Приём пищи: заключённые садятся за столы в столовой.
- Душ: заключённые пользуются душем.
- Перерыв: свободное время.
- Тревога: ставит другие события на паузу (но боты-заложники продолжают им следовать). Вызывается, если заключённого нет в камере ночью, пропущена перекличка, в тюрьме обнаружены повреждения или охранник просыпается без своего ключа. После тревоги все повреждения карты устраняются. Тревога поднимается для конкретного списка персонажей в зависимости от причины её начала.
- Пропуск заключённым любого события распорядка дня повышает его подозрительность или вызывает тревогу.

### 🤖 Искусственный интеллект NPC

- Типы NPC: заключённые, охранники, медики.
- Реализовано через State Trees, Smart Objects, AI Controllers, Detour Crowd Avoidance, Navigation Mesh и NavLinkProxies.
- Поведение зависит от роли и текущего события.
- Охранники:
  - Следуют распорядку дня.
  - Патрулируют, следя за заключёнными и ища повреждения тюрьмы.
  - Атакуют заключённых с высоким уровнем подозрительности.
  - Проверяют камеры на отсутствующих заключённых ночью.
- Медики:
  - Реагируют на персонажей без сознания и несут их в лазарет.
- Заключённые:
  - Следуют распорядку дня.
  - Используют кровати, душ и другие объекты.
- В пассивном состоянии перемещаются по разрешённым зонам, поделенным для разных типов NPC благодаря NavArea.

### 🥈 Побег

- Игра завершается, когда один из игроков успешно сбегает.
- Условие побега: выйти за пределы тюрьмы через разрушенную зону, оставшись незамеченным.

### 🖥️ UI

UI, реализованный через Common UI, предоставляет:
- Главное меню: запуск игры, настройки, выход.
- Меню паузы: продолжить, настройки, выход в главное меню.
- Меню настроек: громкость, качество графики.
- HUD игрока: характеристики, время, текущее событие, отметка присутствия, инвентарь.
- Меню персонажа: имя, одежда, крафт.
- Экран победы: отображает время побега.
- Имена игроков видны над головами (текст повёрнут к камере).

Также имеются прочие различные игровые меню, например, меню открытой тумбочки или чужого инвентаря.

### 💾 Система сохранений

- Реализована через сериализацию полей, помеченных как `UPROPERTY(SaveGame)`.
- Включает в себя:
  - Автозагрузку при создании игры хостом.
  - Автосохранение каждые несколько секунд и при окончании игры.
  - Автоматическое сохранение конкретного игрока при его выходе.
  - Загрузку присоединившегося игрока при его входе.
- Сохраняются:
  - Все персонажи, их атрибуты и состояния.
  - Все инвентари, тумбочки и лежащие на карте предметы.
  - Динамические повреждения карты.
  - Активное событие и его данные.
  - Текущее игровое время.
- Поддерживаются онлайн- и офлайн-игроки, а также боты.
- Каждый персонаж имеет уникальный идентификатор.
- Поддерживается передача файлов сохранений между хостами.

### 🌐 Сетевой режим

- Реализовано через OnlineInterface, OnlineSessions и Steam Sockets.
- Поддерживает до 4 игроков одновременно.
- Хост создаёт игру и владеет сохранением. Клиент подключается через приглашение в Steam.

## 👥 Команда проекта

### Владислав Семчук

*(также известный как Driver2579)*

[LinkedIn](https://www.linkedin.com/in/vladislav-semchuk-a2b944203/)

Роль: Тимлидер, проджект-менеджер, программист.

Вклад:
- Проектирование общей архитектуры игры.
- Планирование задач и сроков.
- Распределение ролей.
- Контроль качества кода.
- Интеграция Gameplay Ability System (GAS).
- Система управления персонажем через GAS.
- Передвижение персонажа с использованием Mover и GAS.
- Система модульных компонентов Gameplay Abilities.
- Поддержка разных PlayerStates для разных контроллеров для модульной настройки GAS и не только для разных типов персонажей.
- Система сохранений и ее поддержка всеми компонентами проекта.
- Идентификация персонажей.
- Система владельцев объектов через компонент.
- Система игрового времени.
- Система игровых событий и сами игровые события.
- Система отметки на игровых событиях.
- Система смены дня и ночи.
- Компонент для включения/выключения света в зависимости от текущего игрового времени.
- Интеграция Steam и поддержка P2P-подключения.
- Телепортация персонажа в кровать при первом заходе в игру.
- Спавн ботов и кастомный PlayerStart с фильтром по тегам персонажа.
- Создание основной логики NPC и AI ботов (патрулирование, тревоги, поведение).
- Искусственный интеллект (ИИ).
- Контрабандные предметы.
- Металлодетектор.
- Предметы одежды и смена модели персонажа.
- Использование предметов на отдельную кнопку.
- Расходуемые предметы.
- Предмет куклы для кровати.
- UI основного HUD игрока, кроме инвентаря (характеристики, время, текущее событие, отметка присутствия).
- UI главного меню без дизайна.
- Функцоинал UI меню паузы (открытие меню настроек и переход в главное меню).
- Система и UI меню настроек.
- UI имени над головой персонажа.
- Компонент для разрушаемости объектов.
- Менеджер музыки для игровых событий.
- Поиск музыки.
- Прочая помощь остальным участникам команды.

### Андрей Шеленговский

[LinkedIn](https://www.linkedin.com/in/andrey-shelengovskiy/)

Роль: программист.

Вклад:
- Система инвентаря.
- Система крафта.
- Тумбочки.
- Обыск персонажей.
- Система для UI.
- UI инвентаря в HUD игрока.
- UI меню персонажа (имя, одежда, крафт).
- UI тумбочек.
- UI обыска персонажей.
- Стили для UI главного меню.
- Стили для UI меню настроек.
- UI меню паузы (возможность открыть и закрыть, и стили).
- Все объекты для взаимодействия (кровати, больничные койки, душевые, сиденья и т. д.).
- Боевая система.
- Предметы инструментов и их использование для ломания объектов через компонент разрушаемости.
- Предметы оружия.
- Двери и ключи.
- Прочность предметов.
- Подбор персонажей.
- Триггер победы игроков для побега.
- UI победы игроков.
- Анимации (поиск и реализация).
- Звуки (поиск и реализация).

### Юлия Врублёвская

Роль: дизайнер.

Вклад:
- Level-дизайн.
- 3D-модели.
- UI-дизайн.
- Поиск музыки.
- Поиск звуков.
