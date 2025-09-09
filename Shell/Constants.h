#pragma once

// --- [���ø����̼� ���] ---

// �۾� ǥ���� ���� ���
constexpr int TASKBAR_HEIGHT = 48;           // �۾� ǥ������ ����

// �۾� ǥ���� ȿ�� ���
constexpr bool GRADIENT_EFFECT_ENABLED = true; // [�߰�] �׶��̼� ȿ�� ��� ����
constexpr bool ACRYLIC_EFFECT_ENABLED = false; // [����] ��ũ�� ȿ���� ���ÿ� ������� �ʵ��� ��Ȱ��ȭ
constexpr int  ACRYLIC_EFFECT_ALPHA = 255;    // ���� (0 ~ 255, �������� ����)
constexpr int  ACRYLIC_EFFECT_RED = 255;       // ���� Red
constexpr int  ACRYLIC_EFFECT_GREEN = 255;     // ���� Green
constexpr int  ACRYLIC_EFFECT_BLUE = 255;      // ���� Blue

// ���� ��ư ���� ���
constexpr int START_BUTTON_SIZE = 32;        // ���� ��ư �������� ũ��
constexpr int START_BUTTON_HOVER_SIZE = 40;  // ���� ��ư ȣ�� ������ ũ��

// ���� �޴� ���� ���
constexpr int START_MENU_WIDTH = 480;        // ���� �޴��� �ʺ�
constexpr int START_MENU_HEIGHT = 560;       // ���� �޴��� ����
constexpr int START_MENU_ITEM_WIDTH = 96;    // ���� �޴� �������� �ʺ�
constexpr int START_MENU_ITEM_HEIGHT = 96;   // ���� �޴� �������� ����
constexpr int START_MENU_ITEM_PADDING = 8;   // ���� �޴� ������ ���� ����
constexpr int START_MENU_ICON_SIZE = 32;     // ���� �޴� �������� ũ��

// �ִϸ��̼� ���� ���
constexpr int ANIMATION_TIMER_ID = 1;        // �ִϸ��̼� Ÿ�̸� ID
constexpr int ANIMATION_STEP_MS = 10;        // �ִϸ��̼� Ÿ�̸� ���� (ms)
constexpr int ANIMATION_DURATION_MS = 100;   // �ִϸ��̼� �� ���� �ð� (ms)