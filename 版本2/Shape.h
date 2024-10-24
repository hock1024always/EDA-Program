#pragma once
#ifndef SHAPE_H
#define SHAPE_H

// ���� ShapeType ö�����ͣ���ʾͼ������
enum class ShapeType { OnPin, OffPin, NotGate, OrGate, AndGate };

// ���� Shape �ṹ�壬��ʾͼ�ε�λ�ú�����
struct Shape {
    ShapeType type; // ͼ�ε�����
    int x, y;       // ͼ�ε�λ������
};

// ���� Line �ṹ�壬��ʾ�����������յ�
struct Line {
    int startX, startY, endX, endY; // �����������յ�����
};

#endif // SHAPE_H


