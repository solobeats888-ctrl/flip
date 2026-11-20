#include "Theme.h"
#include <BinaryData.h>

namespace flipper::theme
{
static juce::Typeface::Ptr bebas()
{
    static juce::Typeface::Ptr tf = juce::Typeface::createSystemTypefaceFor (
        FlipperBinary::BebasNeueRegular_ttf, (size_t) FlipperBinary::BebasNeueRegular_ttfSize);
    return tf;
}

// Las alturas que recibe la API son ALTURA DE MAYUSCULA (como se mide en la referencia).
static float capRatio()
{
    static float ratio = []
    {
        juce::GlyphArrangement ga;
        ga.addLineOfText (juce::Font (juce::FontOptions (bebas()).withHeight (100.0f)), "H", 0.0f, 0.0f);
        juce::Path p; ga.createPath (p);
        const float h = p.getBounds().getHeight();
        return h > 1.0f ? h / 100.0f : 0.7f;
    }();
    return ratio;
}

juce::Font font (float capHeight)
{
    return juce::Font (juce::FontOptions (bebas()).withHeight (capHeight / capRatio()));
}

juce::Image image (const char* name)
{
    int size = 0;
    if (auto* data = FlipperBinary::getNamedResource (name, size))
        return juce::ImageCache::getFromMemory (data, size);
    jassertfalse; // asset faltante
    return {};
}

float textWidth (const juce::String& text, float height)
{
    return juce::GlyphArrangement::getStringWidth (font (height), text) * (1.0f + 0.06f);
}

float fitHeight (const juce::String& text, float maxWidth, float maxHeight)
{
    const float w = textWidth (text, maxHeight);
    return w <= maxWidth ? maxHeight : maxHeight * maxWidth / w;
}

void drawPrintedText (juce::Graphics& g, const juce::String& text, float cx, float cy,
                      float height, float angle, juce::Colour colour,
                      juce::Colour wearColour, float wear, bool shadow)
{
    juce::Graphics::ScopedSaveState save (g);
    g.addTransform (juce::AffineTransform::rotation (juce::degreesToRadians (angle), cx, cy));

    // contornos de los glifos, con trazo extra para el peso grueso de las letras impresas
    juce::GlyphArrangement ga;
    ga.addLineOfText (font (height), text, 0.0f, 0.0f);
    juce::Path p; ga.createPath (p);
    const float w = juce::GlyphArrangement::getStringWidth (font (height), text);
    p.applyTransform (juce::AffineTransform::translation (cx - w * 0.5f, cy + height * 0.5f));
    const juce::PathStrokeType weight (height < 20.0f ? height * 0.035f : juce::jmin (2.4f, height * 0.06f),
                                       juce::PathStrokeType::mitered);

    if (shadow)
    {
        juce::Path s (p); s.applyTransform (juce::AffineTransform::translation (1.3f, 1.6f));
        g.setColour (juce::Colours::black.withAlpha (0.5f));
        g.fillPath (s); g.strokePath (s, weight);
    }
    g.setColour (colour);
    g.fillPath (p);
    g.strokePath (p, weight);

    // desgaste de impresion: motas pequenas, siempre iguales para el mismo texto
    if (wear > 0.0f)
    {
        juce::Graphics::ScopedSaveState clip (g);
        g.reduceClipRegion (p);
        juce::Random r ((juce::int64) text.hashCode64());
        const int specks = (int) (w * wear * 0.5f);
        for (int i = 0; i < specks; ++i)
        {
            const float x = cx - w * 0.5f + r.nextFloat() * w;
            const float y = cy - height * 0.5f + r.nextFloat() * height;
            const float s = 0.6f + r.nextFloat() * height * 0.07f;
            g.setColour (wearColour.withAlpha (0.3f + r.nextFloat() * 0.55f));
            g.fillEllipse (x, y, s * (1.0f + r.nextFloat()), s);
        }
    }
}
}
