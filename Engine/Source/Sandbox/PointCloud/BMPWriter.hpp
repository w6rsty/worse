#pragma once
#include <vector>
#include <cstdint>
#include <fstream>

// BMP图片生成器类
class BMPWriter
{
private:
    struct BMPHeader
    {
        std::uint16_t type = 0x4D42;        // 'BM'
        std::uint32_t size;                 // 文件大小
        std::uint32_t reserved   = 0;       // 保留字段
        std::uint32_t offset     = 54;      // 数据偏移量
        std::uint32_t headerSize = 40;      // 信息头大小
        std::uint32_t width;                // 图像宽度
        std::uint32_t height;               // 图像高度
        std::uint16_t planes          = 1;  // 颜色平面数
        std::uint16_t bitsPerPixel    = 24; // 每像素位数
        std::uint32_t compression     = 0;  // 压缩类型
        std::uint32_t imageSize       = 0;  // 图像大小
        std::uint32_t xPixelsPerMeter = 0;
        std::uint32_t yPixelsPerMeter = 0;
        std::uint32_t colorsUsed      = 0;
        std::uint32_t colorsImportant = 0;
    };

    struct Color
    {
        std::uint8_t r, g, b;
        Color(std::uint8_t red = 0, std::uint8_t green = 0,
              std::uint8_t blue = 0)
            : r(red), g(green), b(blue)
        {
        }
    };

    std::vector<Color> pixels;
    std::uint32_t width;
    std::uint32_t height;
    std::uint32_t padding;

public:
    BMPWriter(std::uint32_t w, std::uint32_t h,
              Color backgroundColor = Color(255, 255, 255),
              std::uint32_t pad     = 0)
        : width(w + 2 * pad), height(h + 2 * pad), padding(pad),
          pixels((w + 2 * pad) * (h + 2 * pad), backgroundColor)
    {
    }

    // 从bool数据构造
    BMPWriter(std::vector<bool> const& data, std::uint32_t w, std::uint32_t h,
              ::uint32_t pad = 0)
        : width(w + 2 * pad), height(h + 2 * pad), padding(pad),
          pixels((w + 2 * pad) * (h + 2 * pad), Color(0, 0, 0))
    {
        // Fill the center area with data, leave padding area as falseColor
        for (std::uint32_t y = 0; y < h && y < h; ++y)
        {
            for (std::uint32_t x = 0; x < w && x < w; ++x)
            {
                std::uint32_t dataIndex = y * w + x;
                if (dataIndex < data.size())
                {
                    Color color =
                        data[dataIndex] ? Color(255, 255, 255) : Color(0, 0, 0);
                    setPixelDirect(x + padding, y + padding, color);
                }
            }
        }
    }

    // 设置像素颜色 (考虑padding偏移)
    void setPixel(std::uint32_t x, std::uint32_t y, Color color)
    {
        setPixelDirect(x + padding, y + padding, color);
    }

    // 直接设置像素颜色 (不考虑padding偏移)
    void setPixelDirect(std::uint32_t x, std::uint32_t y, Color color)
    {
        if (x < width && y < height)
        {
            pixels[y * width + x] = color;
        }
    }

    // 获取像素颜色 (考虑padding偏移)
    Color getPixel(std::uint32_t x, std::uint32_t y) const
    {
        return getPixelDirect(x + padding, y + padding);
    }

    // 直接获取像素颜色 (不考虑padding偏移)
    Color getPixelDirect(std::uint32_t x, std::uint32_t y) const
    {
        if (x < width && y < height)
        {
            return pixels[y * width + x];
        }
        return Color();
    }

    // 获取内容区域的宽度和高度 (不包括padding)
    std::uint32_t getContentWidth() const
    {
        return width - 2 * padding;
    }

    std::uint32_t getContentHeight() const
    {
        return height - 2 * padding;
    }

    // 绘制点
    void drawPoint(std::uint32_t x, std::uint32_t y,
                   Color color = Color(255, 0, 0), std::uint32_t size = 1)
    {
        for (std::uint32_t dy = 0; dy < size; ++dy)
        {
            for (std::uint32_t dx = 0; dx < size; ++dx)
            {
                setPixel(x + dx, y + dy, color);
            }
        }
    }

    // 绘制线段 (Bresenham算法)
    void drawLine(std::uint32_t x0, std::uint32_t y0, std::uint32_t x1,
                  std::uint32_t y1, Color color = Color(255, 0, 0))
    {
        int dx  = std::abs(static_cast<int>(x1) - static_cast<int>(x0));
        int dy  = std::abs(static_cast<int>(y1) - static_cast<int>(y0));
        int sx  = x0 < x1 ? 1 : -1;
        int sy  = y0 < y1 ? 1 : -1;
        int err = dx - dy;

        int x = static_cast<int>(x0);
        int y = static_cast<int>(y0);

        while (true)
        {
            setPixel(static_cast<std::uint32_t>(x),
                     static_cast<std::uint32_t>(y),
                     color);

            if (x == static_cast<int>(x1) && y == static_cast<int>(y1))
                break;

            int e2 = 2 * err;
            if (e2 > -dy)
            {
                err -= dy;
                x += sx;
            }
            if (e2 < dx)
            {
                err += dx;
                y += sy;
            }
        }
    }

    // 绘制矩形框
    void drawRectangle(std::uint32_t x, std::uint32_t y, std::uint32_t w,
                       std::uint32_t h, Color color = Color(255, 0, 0),
                       bool filled = false)
    {
        if (filled)
        {
            // 填充矩形
            for (std::uint32_t dy = 0; dy < h; ++dy)
            {
                for (std::uint32_t dx = 0; dx < w; ++dx)
                {
                    setPixel(x + dx, y + dy, color);
                }
            }
        }
        else
        {
            // 绘制矩形边框
            // 上边
            drawLine(x, y, x + w - 1, y, color);
            // 下边
            drawLine(x, y + h - 1, x + w - 1, y + h - 1, color);
            // 左边
            drawLine(x, y, x, y + h - 1, color);
            // 右边
            drawLine(x + w - 1, y, x + w - 1, y + h - 1, color);
        }
    }

    // 绘制圆形
    void drawCircle(std::uint32_t centerX, std::uint32_t centerY,
                    std::uint32_t radius, Color color = Color(255, 0, 0),
                    bool filled = false)
    {
        int x   = static_cast<int>(radius);
        int y   = 0;
        int err = 0;

        while (x >= y)
        {
            if (filled)
            {
                // 填充圆形
                drawLine(centerX - x,
                         centerY + y,
                         centerX + x,
                         centerY + y,
                         color);
                drawLine(centerX - x,
                         centerY - y,
                         centerX + x,
                         centerY - y,
                         color);
                drawLine(centerX - y,
                         centerY + x,
                         centerX + y,
                         centerY + x,
                         color);
                drawLine(centerX - y,
                         centerY - x,
                         centerX + y,
                         centerY - x,
                         color);
            }
            else
            {
                // 绘制圆形边界
                setPixel(centerX + x, centerY + y, color);
                setPixel(centerX + y, centerY + x, color);
                setPixel(centerX - y, centerY + x, color);
                setPixel(centerX - x, centerY + y, color);
                setPixel(centerX - x, centerY - y, color);
                setPixel(centerX - y, centerY - x, color);
                setPixel(centerX + y, centerY - x, color);
                setPixel(centerX + x, centerY - y, color);
            }

            if (err <= 0)
            {
                y += 1;
                err += 2 * y + 1;
            }

            if (err > 0)
            {
                x -= 1;
                err -= 2 * x + 1;
            }
        }
    }

    // 保存为BMP文件
    bool saveToBMP(std::filesystem::path const& filePath) const
    {
        std::ofstream file(filePath, std::ios::binary);
        if (!file.is_open())
        {
            return false;
        }

        // 计算行填充
        std::uint32_t rowPadding = (4 - (width * 3) % 4) % 4;
        std::uint32_t imageSize  = (width * 3 + rowPadding) * height;

        BMPHeader header;
        header.width     = width;
        header.height    = height;
        header.size      = 54 + imageSize;
        header.imageSize = imageSize;

        // 写入文件头
        file.write(reinterpret_cast<const char*>(&header.type), 2);
        file.write(reinterpret_cast<const char*>(&header.size), 4);
        file.write(reinterpret_cast<const char*>(&header.reserved), 4);
        file.write(reinterpret_cast<const char*>(&header.offset), 4);

        // 写入信息头
        file.write(reinterpret_cast<const char*>(&header.headerSize), 4);
        file.write(reinterpret_cast<const char*>(&header.width), 4);
        file.write(reinterpret_cast<const char*>(&header.height), 4);
        file.write(reinterpret_cast<const char*>(&header.planes), 2);
        file.write(reinterpret_cast<const char*>(&header.bitsPerPixel), 2);
        file.write(reinterpret_cast<const char*>(&header.compression), 4);
        file.write(reinterpret_cast<const char*>(&header.imageSize), 4);
        file.write(reinterpret_cast<const char*>(&header.xPixelsPerMeter), 4);
        file.write(reinterpret_cast<const char*>(&header.yPixelsPerMeter), 4);
        file.write(reinterpret_cast<const char*>(&header.colorsUsed), 4);
        file.write(reinterpret_cast<const char*>(&header.colorsImportant), 4);

        // 写入像素数据 (BMP格式是从下到上存储的)
        std::vector<std::uint8_t> padding(rowPadding, 0);
        for (int y = static_cast<int>(height) - 1; y >= 0; --y)
        {
            for (std::uint32_t x = 0; x < width; ++x)
            {
                Color pixel = getPixel(x, static_cast<std::uint32_t>(y));
                // BMP格式是BGR顺序
                file.write(reinterpret_cast<const char*>(&pixel.b), 1);
                file.write(reinterpret_cast<const char*>(&pixel.g), 1);
                file.write(reinterpret_cast<const char*>(&pixel.r), 1);
            }
            // 写入行填充
            if (rowPadding > 0)
            {
                file.write(reinterpret_cast<const char*>(padding.data()),
                           rowPadding);
            }
        }

        file.close();
        return true;
    }

    // 获取图像尺寸 (包括padding)
    std::uint32_t getWidth() const
    {
        return width;
    }
    std::uint32_t getHeight() const
    {
        return height;
    }

    // 获取padding大小
    std::uint32_t getPadding() const
    {
        return padding;
    }
};