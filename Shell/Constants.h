#pragma once

// --- [애플리케이션 상수] ---

// 작업 표시줄 관련 상수
constexpr int TASKBAR_HEIGHT = 48;           // 작업 표시줄의 높이

// 작업 표시줄 효과 상수
constexpr bool GRADIENT_EFFECT_ENABLED = true; // [추가] 그라데이션 효과 사용 여부
constexpr bool ACRYLIC_EFFECT_ENABLED = false; // [수정] 아크릴 효과는 동시에 사용하지 않도록 비활성화
constexpr int  ACRYLIC_EFFECT_ALPHA = 255;    // 투명도 (0 ~ 255, 낮을수록 투명)
constexpr int  ACRYLIC_EFFECT_RED = 255;       // 배경색 Red
constexpr int  ACRYLIC_EFFECT_GREEN = 255;     // 배경색 Green
constexpr int  ACRYLIC_EFFECT_BLUE = 255;      // 배경색 Blue

// 시작 버튼 관련 상수
constexpr int START_BUTTON_SIZE = 32;        // 시작 버튼 아이콘의 크기
constexpr int START_BUTTON_HOVER_SIZE = 40;  // 시작 버튼 호버 영역의 크기

// 시작 메뉴 관련 상수
constexpr int START_MENU_WIDTH = 480;        // 시작 메뉴의 너비
constexpr int START_MENU_HEIGHT = 560;       // 시작 메뉴의 높이
constexpr int START_MENU_ITEM_WIDTH = 96;    // 시작 메뉴 아이템의 너비
constexpr int START_MENU_ITEM_HEIGHT = 96;   // 시작 메뉴 아이템의 높이
constexpr int START_MENU_ITEM_PADDING = 8;   // 시작 메뉴 아이템 간의 간격
constexpr int START_MENU_ICON_SIZE = 32;     // 시작 메뉴 아이콘의 크기

// 애니메이션 관련 상수
constexpr int ANIMATION_TIMER_ID = 1;        // 애니메이션 타이머 ID
constexpr int ANIMATION_STEP_MS = 10;        // 애니메이션 타이머 간격 (ms)
constexpr int ANIMATION_DURATION_MS = 100;   // 애니메이션 총 지속 시간 (ms)