# Direct-connect multiplayer (4J macOS)

Минимальная TCP-реализация мультиплеера поверх существующих
`Socket` / `ClientConnection` / `ServerConnection`.

## Файлы

- `Socket.{h,cpp}` — добавлен TCP-режим (`m_isTcp`, фоновый reader-поток,
  send через `::send()` в `writeWithFlags`), статические хелперы
  `Socket::ConnectTcp`, `Socket::StartTcpListener`, `Socket::StopTcpListener`.
- `RemoteNetworkPlayer.{h,cpp}` — минимальный `INetworkPlayer` для удалённых
  пиров. Регистрируется в глобальном map `smallId -> player`, используется
  `IPlatformNetworkStub::GetPlayerBySmallId` как primary lookup.
- `PlatformNetworkManagerStub.{h,cpp}` — поднимает listener в `_HostGame`,
  останавливает в `_LeaveGame`. Добавлен `s_pRemoteHostOverride` для возврата
  удалённого хоста из `GetHostPlayer()` на клиенте.
- `XboxStubs.cpp` — флаг `_bQNetStubIsHost`, который выключается на клиенте,
  чтобы `g_NetworkManager.IsHost()` возвращал `false`.
- `MacGame.{h,cpp}` — `TemporaryDirectConnectStart(host, port)`: открывает
  TCP, привязывает сокет к локальному игроку, поднимает client-mode флаги и
  запускает обычный `RunNetworkGameThreadProc`.
- `Mac_Minecraft.cpp` — в `main()` читает env-переменную
  `MC_DIRECT_CONNECT=host[:port]` и вызывает direct-connect стартап.

## Использование

### Хост

Запустить игру обычным способом и захостить мир «онлайн»
(в Create World / Load World ставится флаг online).
Listener поднимется автоматически на порту 25565.

Чтобы хостить даже offline-мир (для теста), задать порт:

```sh
MC_LISTEN_PORT=25565 ./Minecraft.Client
```

### Клиент

Запустить второй инстанс с переменной окружения:

```sh
MC_DIRECT_CONNECT=127.0.0.1:25565 ./Minecraft.Client
```

Клиент пропустит главное меню и сразу попытается подключиться. В консоли
будут строки `[TCP] ...` про connect/accept.

## Известные ограничения (Phase 1)

- `RemoteNetworkPlayer::SendData` fallback не реализован; обмен идёт
  через `Socket`-stream напрямую.
- Скины/имена клиента — заглушки (`L"Client"`, `player-<ip>-<port>`).
- Disconnect / переподключение в рамках одного процесса не тестировалось.

## UI-путь для подключения (Phase 2)

- `SelectWorldScreen` получил кнопку **"Multiplayer (Direct Connect)"**
  которая открывает `JoinMultiplayerScreen`.
- `JoinMultiplayerScreen::buttonClicked` парсит `host[:port]` (в т.ч.
  `[ipv6]:port`), сохраняет в `options->lastMpIp`, вызывает
  `app.TemporaryDirectConnectStart(host, port)`.
- Кнопка Connect всегда активна (обход нерабочего `keyPressed` через
  SDL2 на macOS).

## Критические фиксы для стриминга чанков (Phase 3)

1. **Slow-queue index mismatch.** `RemoteNetworkPlayer::GetSessionIndex()`
   раньше возвращал `m_smallId` (=2), но `MinecraftServer`'s cycler
   крутит индекс в `[0, playerCount)`. Теперь возвращает `m_smallId - 1`.
2. **Player enumeration.** `IPlatformNetworkStub::GetPlayerCount()` и
   `GetPlayerByIndex()` включают remote-игроков; добавлены
   `RemoteNetworkPlayer::GetActiveCount()` / `GetByActiveIndex()`.
3. **Primary ClientConnection не тикался.** После `setScreen(nullptr)`
   в `handleMovePlayer` никто не вызывал `connection->tick()`, пакеты
   копились в `incoming` очереди. Фикс: в `ClientConnection::handleLogin`
   биндим `minecraft->localplayers[primary]->connection = this` —
   главный `Minecraft::run_middle` loop ловит его.
4. **LevelRenderer не знал про новый Level.** В `handleLogin` явно
   вызываем `levelRenderer->setLevel(primary, level)` +
   `particleEngine->setLevel(level)` + `cameraTargetPlayer = player`.
5. **Chunks писались в `emptyChunk` sentinel.** Перед `setBlocksAndData`
   в `handleBlockRegionUpdate` вызываем `dimensionLevel->setChunkVisible(xc, zc, true)`
   для всех затронутых чанков, чтобы `MultiPlayerChunkCache` реально
   аллоцировал `LevelChunk`, а не возвращал общий пустой.

## Оставшиеся баги (не транспорт)

- **Тёмная зона вокруг спавна.** Skylight не пересчитывается на
  клиенте для full-chunk BRUP.
- **Нет HUD / хотбара.** `UIScene_FullscreenProgress` не закрывается
  — direct-connect обходит `StateChange_AnyToStarting` и
  `e_ProgressCompletion_CloseAllPlayersUIScenes` не триггерится.
- **Не видно других игроков и мобов.** `AddPlayerPacket` отбрасывается
  на клиенте в `ClientConnection::handleAddPlayer` по XUID — у обоих
  процессов дефолтный XUID одинаковый (нет Xbox-профилей).
- **Телепорт обратно при движении.** Сервер делает position correction
  потому что client-side player не синхронизирован с ServerPlayer на
  сервере.
- **Маленькая прорисовка.** View distance по умолчанию (`mc->options->viewDistance`).

## Полезные логи

- `[TCP] ...` — транспорт / handshake.
- `[TCP] fd=N sent/recv total=NNN bytes` — throughput в Socket.
- `[TCP] handleBlockRegionUpdate #N ... level=0x...` — BRUP на клиенте.
- `[TCP] Bound LevelRenderer[pad=N] to level=0x...` — рендерер привязан.
- `[TCP] handleMovePlayer #N -> (x,y,z)` — позиция игрока после teleport.
