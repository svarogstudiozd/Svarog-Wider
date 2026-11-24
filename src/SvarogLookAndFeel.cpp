#include "SvarogLookAndFeel.h"

juce::Typeface::Ptr SvarogLookAndFeel::getInterBold()
{
    static auto tf = juce::Typeface::createSystemTypefaceFor (
        BinaryData::Inter_18ptBold_ttf,
        BinaryData::Inter_18ptBold_ttfSize);
    return tf;
}

juce::Typeface::Ptr SvarogLookAndFeel::getInterRegular()
{
    static auto tf = juce::Typeface::createSystemTypefaceFor (
        BinaryData::Inter_18ptRegular_ttf,
        BinaryData::Inter_18ptRegular_ttfSize);
    return tf;
}

juce::Font SvarogLookAndFeel::getLabelFont (juce::Label& label)
{
    const auto current = label.getFont();
    const auto style = current.getTypefaceStyle();
    const bool isBold = style.containsIgnoreCase ("bold")
                     || style.containsIgnoreCase ("black")
                     || style.containsIgnoreCase ("heavy");

    const auto base = isBold ? getInterBold() : getInterRegular();
    return juce::Font (juce::FontOptions (base).withHeight (current.getHeight()));
}

juce::Font SvarogLookAndFeel::getTextButtonFont (juce::TextButton& button, int buttonHeight)
{
    const auto fontSizeVar = button.getProperties()["fontSize"];
    const float fontSize = fontSizeVar.isVoid()
        ? juce::jmin (15.0f, (float) buttonHeight * 0.55f)
        : (float) fontSizeVar;

    return juce::Font (juce::FontOptions (getInterBold()).withHeight (fontSize));
}


void SvarogLookAndFeel::drawButtonText (juce::Graphics& g,
                                       juce::TextButton& button,
                                       bool /*shouldDrawButtonAsHighlighted*/,
                                       bool /*shouldDrawButtonAsDown*/)
{
    const auto fontSizeVar = button.getProperties()["fontSize"];
    const float fontSize = fontSizeVar.isVoid() ? 15.0f : (float) fontSizeVar;

    juce::Font font (juce::FontOptions (getInterBold()).withHeight (fontSize));
    g.setFont (font);

    const auto normalColour = button.getToggleState()
        ? Theme::textPrimary
        : button.findColour (juce::TextButton::textColourOffId);

    const auto textColour = button.isEnabled()
        ? normalColour
        : normalColour.withMultipliedAlpha (0.4f);

    const int hPad = 4;
    const int vPad = 0;

    const auto nudgeVar = button.getProperties()["textNudgeY"];
    const int textNudgeY = nudgeVar.isVoid() ? 0 : (int) nudgeVar;

    const auto textArea = button.getLocalBounds()
                                .reduced (hPad, vPad)
                                .translated (0, textNudgeY);

    if (button.isEnabled() && ! button.getToggleState())
    {
        g.setColour (juce::Colours::black.withAlpha (0.5f));
        g.drawFittedText (button.getButtonText(),
                          textArea.translated (0, 1),
                          juce::Justification::centred,
                          1);
    }

    g.setColour (textColour);
    g.drawFittedText (button.getButtonText(),
                      textArea,
                      juce::Justification::centred,
                      1);
}

void SvarogLookAndFeel::drawControlShadow (juce::Graphics& g, juce::Rectangle<float> bounds) const
{
    juce::Path shadowPath;
    shadowPath.addRoundedRectangle (bounds.translated (0.0f, (float) Shadow::cardOffsetY),
                                    Radius::control);
    juce::DropShadow drop (juce::Colours::black.withAlpha (Shadow::cardAlpha),
                           Shadow::cardBlur, { 0, 0 });
    drop.drawForPath (g, shadowPath);
}

void SvarogLookAndFeel::drawActiveControlShadow (juce::Graphics& g, juce::Rectangle<float> bounds) const
{
    juce::Path glowPath;
    glowPath.addRoundedRectangle (bounds, Radius::control);
    juce::DropShadow glow (Theme::accentRedBright.withAlpha (0.55f), 10, { 0, 0 });
    glow.drawForPath (g, glowPath);

    drawControlShadow (g, bounds);
}

void SvarogLookAndFeel::drawControlGlow (juce::Graphics& g, juce::Rectangle<float> bounds) const
{
    juce::Path glowPath;
    glowPath.addRoundedRectangle (bounds, Radius::control);
    juce::DropShadow glow (Theme::accentRedBright.withAlpha (0.55f), 10, { 0, 0 });
    glow.drawForPath (g, glowPath);
}

void SvarogLookAndFeel::drawTileShadow (juce::Graphics& g, juce::Rectangle<float> bounds) const
{
    juce::Path shadowPath;
    shadowPath.addRoundedRectangle (bounds.translated (0.0f, (float) Shadow::tileOffsetY),
                                    Radius::control);
    juce::DropShadow drop (juce::Colours::black.withAlpha (Shadow::tileAlpha),
                           Shadow::tileBlur, { 0, 0 });
    drop.drawForPath (g, shadowPath);
}

void SvarogLookAndFeel::drawPanelShadow (juce::Graphics& g, juce::Rectangle<float> bounds) const
{
    juce::Path shadowPath;
    shadowPath.addRoundedRectangle (bounds.translated (0.0f, (float) Shadow::panelOffsetY),
                                    Radius::panel);
    juce::DropShadow drop (juce::Colours::black.withAlpha (Shadow::panelAlpha),
                           Shadow::panelBlur, { 0, 0 });
    drop.drawForPath (g, shadowPath);
}

void SvarogLookAndFeel::drawPanel (juce::Graphics& g,
                                  juce::Rectangle<float> bounds,
                                  juce::Colour fill) const
{
    g.setColour (fill);
    g.fillRoundedRectangle (bounds, Radius::panel);

    {
        const auto r = Radius::panel;
        auto pathBounds = bounds.reduced (1.0f);
        const float pathR = juce::jmax (0.0f, r - 1.0f);

        juce::Path inner;
        inner.addRoundedRectangle (pathBounds, pathR);

        juce::Graphics::ScopedSaveState saveInner (g);
        // Keep the highlight clipped to a shallow 4px top strip so the
        // curved corners don't get a heavy white rim
        juce::Rectangle<float> topBand (bounds.getX(), bounds.getY(), bounds.getWidth(), 4.0f);
        juce::Path band;
        band.addRectangle (topBand);
        g.reduceClipRegion (band, {});

        juce::Path outline;
        juce::PathStrokeType (1.0f, juce::PathStrokeType::curved, juce::PathStrokeType::butt)
            .createStrokedPath (outline, inner);

        // Softer corner opacity (0.12f) with a gentle top-centre highlight (0.28f)
        juce::ColourGradient fade (
            juce::Colours::white.withAlpha (0.0f), bounds.getX(), 0.0f,
            juce::Colours::white.withAlpha (0.0f), bounds.getRight(), 0.0f, false);
        fade.addColour (0.02, juce::Colours::white.withAlpha (0.18f));
        fade.addColour (0.50, juce::Colours::white.withAlpha (0.38f));
        fade.addColour (0.98, juce::Colours::white.withAlpha (0.18f));

        g.setGradientFill (fade);
        g.fillPath (outline);
    }

    g.setColour (juce::Colours::black.withAlpha (0.85f));
    g.drawRoundedRectangle (bounds.reduced (0.5f), Radius::panel, 1.0f);
}

void SvarogLookAndFeel::drawWell (juce::Graphics& g,
                                 juce::Rectangle<float> bounds,
                                 juce::Colour fill) const
{
    const float wellRadius = juce::jmax (Radius::well, 6.0f);

    g.setColour (fill);
    g.fillRoundedRectangle (bounds, wellRadius);

    {
        juce::Graphics::ScopedSaveState save (g);
        juce::Path clip;
        clip.addRoundedRectangle (bounds, wellRadius);
        g.reduceClipRegion (clip, {});

        const float topFade = 14.0f;
        const float bottomFade = 12.0f;
        const float sideFade = 10.0f;

        {
            juce::ColourGradient grad (
                juce::Colours::black.withAlpha (0.0f), bounds.getX(), bounds.getY(),
                juce::Colours::black.withAlpha (0.0f), bounds.getX(), bounds.getY() + topFade, false);
            grad.addColour (0.15, juce::Colours::black.withAlpha (0.60f));
            grad.addColour (0.40, juce::Colours::black.withAlpha (0.35f));
            grad.addColour (0.70, juce::Colours::black.withAlpha (0.20f));
            g.setGradientFill (grad);
            g.fillRect (bounds.withHeight (topFade + 1.0f));
        }
        {
            juce::ColourGradient grad (
                juce::Colours::black.withAlpha (0.0f), bounds.getX(), bounds.getBottom(),
                juce::Colours::black.withAlpha (0.0f), bounds.getX(), bounds.getBottom() - bottomFade, false);
            grad.addColour (0.3, juce::Colours::black.withAlpha (0.30f));
            grad.addColour (0.7, juce::Colours::black.withAlpha (0.08f));
            g.setGradientFill (grad);
            g.fillRect (juce::Rectangle<float> (bounds.getX(), bounds.getBottom() - bottomFade,
                                                bounds.getWidth(), bottomFade));
        }
        {
            juce::ColourGradient grad (
                juce::Colours::black.withAlpha (0.0f), bounds.getX(), bounds.getY(),
                juce::Colours::black.withAlpha (0.0f), bounds.getX() + sideFade, bounds.getY(), false);
            grad.addColour (0.3, juce::Colours::black.withAlpha (0.25f));
            grad.addColour (0.7, juce::Colours::black.withAlpha (0.10f));
            g.setGradientFill (grad);
            g.fillRect (juce::Rectangle<float> (bounds.getX(), bounds.getY(), sideFade, bounds.getHeight()));
        }
        {
            juce::ColourGradient grad (
                juce::Colours::black.withAlpha (0.0f), bounds.getRight(), bounds.getY(),
                juce::Colours::black.withAlpha (0.0f), bounds.getRight() - sideFade, bounds.getY(), false);
            grad.addColour (0.3, juce::Colours::black.withAlpha (0.25f));
            grad.addColour (0.7, juce::Colours::black.withAlpha (0.10f));
            g.setGradientFill (grad);
            g.fillRect (juce::Rectangle<float> (bounds.getRight() - sideFade, bounds.getY(),
                                                sideFade, bounds.getHeight()));
        }
    }

    {
        juce::Graphics::ScopedSaveState save (g);
        juce::Rectangle<float> bottomBand (bounds.getX(), bounds.getBottom() - wellRadius - 2.0f,
                                           bounds.getWidth(), wellRadius + 2.0f);
        juce::Path band;
        band.addRectangle (bottomBand);
        g.reduceClipRegion (band, {});

        juce::Path inner;
        inner.addRoundedRectangle (bounds.reduced (0.5f), juce::jmax (0.0f, wellRadius - 1.0f));

        juce::Path outline;
        juce::PathStrokeType (0.5f, juce::PathStrokeType::curved, juce::PathStrokeType::butt)
            .createStrokedPath (outline, inner);

        juce::ColourGradient fade (
            juce::Colours::white.withAlpha (0.0f), bounds.getX(), 0.0f,
            juce::Colours::white.withAlpha (0.0f), bounds.getRight(), 0.0f, false);
        fade.addColour (0.03, juce::Colours::white.withAlpha (0.10f));
        fade.addColour (0.50, juce::Colours::white.withAlpha (0.05f));
        fade.addColour (0.97, juce::Colours::white.withAlpha (0.10f));
        g.setGradientFill (fade);
        g.fillPath (outline);
    }

    {
        juce::Path borderPath;
        borderPath.addRoundedRectangle (bounds.reduced (1.0f), juce::jmax (0.0f, wellRadius - 1.0f));
        juce::Path outline;
        juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::butt)
            .createStrokedPath (outline, borderPath);

        juce::ColourGradient borderGrad (
            juce::Colours::black.withAlpha (0.10f), bounds.getX(), bounds.getY(),
            juce::Colours::white.withAlpha (0.20f), bounds.getX(), bounds.getBottom(), false);
        borderGrad.addColour (0.0, juce::Colours::white.withAlpha (0.10f));
        g.setGradientFill (borderGrad);
        g.fillPath (outline);
    }
}

void SvarogLookAndFeel::drawRaisedControl (juce::Graphics& g,
                                          juce::Rectangle<float> bounds,
                                          juce::Colour base,
                                          bool isHighlighted,
                                          bool isDown,
                                          bool isGlowing,
                                          float cornerRadius) const
{
    if (isGlowing)
    {
        const auto centre = bounds.getCentre();
        const auto radius = juce::jmax (bounds.getWidth(), bounds.getHeight()) * 0.6f;
        juce::ColourGradient glow (juce::Colour (0xffc84040), centre.x, centre.y,
                                    base.darker (0.30f), centre.x + radius, centre.y, true);
        g.setGradientFill (glow);
        g.fillRoundedRectangle (bounds, cornerRadius);
    }
    else
    {
        juce::Colour top, bottom;
        if (isDown)              { top = base.darker (0.08f); bottom = base.darker (0.18f); }
        else if (isHighlighted)  { top = base.brighter (0.20f); bottom = base.darker (0.12f); }
        else                     { top = base.brighter (0.14f); bottom = base.darker (0.18f); }

        // 1. Vertical base gradient
        juce::ColourGradient grad (top, bounds.getX(), bounds.getY(),
                                    bottom, bounds.getX(), bounds.getBottom(), false);
        g.setGradientFill (grad);
        g.fillRoundedRectangle (bounds, cornerRadius);

        // 2. Horizontal side darkening (only for enabled, non-grey buttons)
        const bool isEnabledButton = base.getSaturation() > 0.1f;
        if (isEnabledButton)
        {
            juce::Graphics::ScopedSaveState save (g);
            juce::Path clip;
            clip.addRoundedRectangle (bounds, cornerRadius);
            g.reduceClipRegion (clip, {});

            juce::Colour sideColour   = base.darker (0.90f);
            juce::Colour centreColour = base;

            juce::ColourGradient hGrad (
                sideColour,
                bounds.getX(), bounds.getY(),
                sideColour,
                bounds.getRight(), bounds.getY(),
                false);

            hGrad.addColour (0.10f, sideColour.interpolatedWith (centreColour, 0.30f));
            hGrad.addColour (0.40f, centreColour);
            hGrad.addColour (0.60f, centreColour);
            hGrad.addColour (0.90f, sideColour.interpolatedWith (centreColour, 0.30f));

            hGrad.multiplyOpacity (0.95f);

            g.setGradientFill (hGrad);
            g.fillRect (bounds);
        }
    }

    if (bounds.getHeight() > 22.0f)
    {
        const auto r = cornerRadius;
        auto pathBounds = bounds.reduced (1.0f);
        const float pathR = juce::jmax (0.0f, r - 1.0f);
        juce::Path inner;
        inner.addRoundedRectangle (pathBounds, pathR);

        juce::Graphics::ScopedSaveState saveInner (g);
        juce::Rectangle<float> topBand (bounds.getX(), bounds.getY(), bounds.getWidth(), 5.0f);
        juce::Path band;
        band.addRectangle (topBand);
        g.reduceClipRegion (band, {});

        juce::Path outline;
        juce::PathStrokeType (1.0f, juce::PathStrokeType::curved, juce::PathStrokeType::butt)
            .createStrokedPath (outline, inner);

        juce::ColourGradient fade (
            juce::Colours::white.withAlpha (0.0f), bounds.getX(), 0.0f,
            juce::Colours::white.withAlpha (0.0f), bounds.getRight(), 0.0f, false);
        fade.addColour (0.05, juce::Colours::white.withAlpha (0.30f));
        fade.addColour (0.50, juce::Colours::white.withAlpha (0.45f));
        fade.addColour (0.95, juce::Colours::white.withAlpha (0.30f));
        g.setGradientFill (fade);
        g.fillPath (outline);
    }

    g.setColour (juce::Colours::black.withAlpha (0.85f));
    g.drawRoundedRectangle (bounds.reduced (0.5f), cornerRadius, 1.0f);

    // Bright 1px highlight along the top edge, drawn AFTER the border
    // so it's visible as a crisp line right inside the top curve.
    {
        juce::Graphics::ScopedSaveState save (g);
        juce::Path clip;
        clip.addRoundedRectangle (bounds.reduced (1.0f), cornerRadius);
        g.reduceClipRegion (clip, {});

        g.setColour (juce::Colours::white.withAlpha (0.25f));
        g.fillRect (juce::Rectangle<float> (bounds.getX() + cornerRadius,
                                             bounds.getY() + 0.2f,
                                             bounds.getWidth() - cornerRadius * 2.0f,
                                             1.0f));
    }
}


void SvarogLookAndFeel::drawRecessedControl (juce::Graphics& g,
                                            juce::Rectangle<float> bounds,
                                            juce::Colour fill,
                                            float cornerRadius) const
{
    g.setColour (fill);
    g.fillRoundedRectangle (bounds, cornerRadius);

    // ---- Inner shadows (clipped to the well interior) ----
    {
        juce::Graphics::ScopedSaveState save (g);
        juce::Path clip;
        clip.addRoundedRectangle (bounds, cornerRadius);
        g.reduceClipRegion (clip, {});

        constexpr float topFade    = 9.0f;
        constexpr float bottomFade = 6.0f;
        constexpr float sideFade   = 6.0f;

        // Top shadow
        {
            juce::ColourGradient grad (juce::Colours::black.withAlpha (0.55f),
                                        bounds.getX(), bounds.getY(),
                                        juce::Colours::black.withAlpha (0.0f),
                                        bounds.getX(), bounds.getY() + topFade, false);
            g.setGradientFill (grad);
            g.fillRect (bounds.withHeight (topFade + 1.0f));
        }
        // Bottom shadow
        {
            juce::ColourGradient grad (juce::Colours::black.withAlpha (0.30f),
                                        bounds.getX(), bounds.getBottom(),
                                        juce::Colours::black.withAlpha (0.0f),
                                        bounds.getX(), bounds.getBottom() - bottomFade, false);
            g.setGradientFill (grad);
            g.fillRect (juce::Rectangle<float> (bounds.getX(), bounds.getBottom() - bottomFade,
                                                bounds.getWidth(), bottomFade));
        }
        // Left shadow
        {
            juce::ColourGradient grad (juce::Colours::black.withAlpha (0.30f),
                                        bounds.getX(), bounds.getY(),
                                        juce::Colours::black.withAlpha (0.0f),
                                        bounds.getX() + sideFade, bounds.getY(), false);
            g.setGradientFill (grad);
            g.fillRect (juce::Rectangle<float> (bounds.getX(), bounds.getY(),
                                                sideFade, bounds.getHeight()));
        }
        // Right shadow
        {
            juce::ColourGradient grad (juce::Colours::black.withAlpha (0.30f),
                                        bounds.getRight(), bounds.getY(),
                                        juce::Colours::black.withAlpha (0.0f),
                                        bounds.getRight() - sideFade, bounds.getY(), false);
            g.setGradientFill (grad);
            g.fillRect (juce::Rectangle<float> (bounds.getRight() - sideFade, bounds.getY(),
                                                sideFade, bounds.getHeight()));
        }
    }

    // ---- Border (drawn OUTSIDE the clip, on top of the shadows) ----
    {
        juce::Path borderPath;
        borderPath.addRoundedRectangle (bounds.reduced (0.5f), cornerRadius);

        juce::Path outline;
        juce::PathStrokeType (1.3f,
                              juce::PathStrokeType::curved,
                              juce::PathStrokeType::butt)
            .createStrokedPath (outline, borderPath);

        juce::ColourGradient borderGrad (
            juce::Colours::white.withAlpha (0.10f),
            bounds.getX(), bounds.getY(),
            juce::Colours::white.withAlpha (0.45f),
            bounds.getX(), bounds.getBottom(),
            false);

        borderGrad.addColour (0.4f, juce::Colours::grey.withAlpha (0.20f));
        borderGrad.addColour (0.8f, juce::Colours::grey.withAlpha (0.26f));

        g.setGradientFill (borderGrad);
        g.fillPath (outline);
    }
}

void SvarogLookAndFeel::drawRotarySlider (juce::Graphics& g,
                                         int x, int y, int width, int height,
                                         float sliderPosProportional,
                                         float rotaryStartAngle,
                                         float rotaryEndAngle,
                                         juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (2.0f);

    auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto centreX = bounds.getCentreX();
    auto centreY = bounds.getCentreY();

    juce::Rectangle<float> wellBounds (centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);

    // ---- Read hover alpha (driven by HoverSlider's timer) ----
    const float hoverAlpha = juce::jlimit (0.0f, 1.0f,
        (float) slider.getProperties().getWithDefault ("hoverAlpha", 0.0f));

    const bool isDragging = (bool) slider.getProperties().getWithDefault ("isDragging", false);
    const float effectiveAlpha = isDragging ? 1.0f : hoverAlpha;

    const float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    const float arcThickness = radius * 0.32f;
    const float arcRadius = radius - (arcThickness * 0.5f) - 2.0f;

    // ---- Well base fill ----
    g.setColour (Theme::surfaceRaised.darker (0.3f));
    g.fillEllipse (wellBounds);

    // ---- Radial inner shadow ----
    {
        juce::Graphics::ScopedSaveState save (g);
        juce::Path clip;
        clip.addEllipse (wellBounds);
        g.reduceClipRegion (clip, {});

        juce::ColourGradient radialShadow (
            juce::Colours::transparentBlack,
            centreX, centreY,
            juce::Colours::black.withAlpha (0.85f),
            centreX + radius, centreY,
            true);
        radialShadow.addColour (0.55, juce::Colours::transparentBlack);
        radialShadow.addColour (0.75, juce::Colours::black.withAlpha (0.35f));
        radialShadow.addColour (0.95, juce::Colours::black.withAlpha (0.75f));
        g.setGradientFill (radialShadow);
        g.fillAll();
    }

    // ---- Bottom highlight lip ----
    {
        juce::Graphics::ScopedSaveState save (g);
        juce::Path clip;
        clip.addEllipse (wellBounds);
        g.reduceClipRegion (clip, {});

        juce::ColourGradient bottomHighlight (
            juce::Colours::transparentWhite,
            centreX, centreY + (radius * 0.4f),
            juce::Colours::white.withAlpha (0.15f),
            centreX, centreY + radius, false);
        g.setGradientFill (bottomHighlight);
        g.fillAll();
    }

    // ---- Border ----
    {
        juce::Path borderPath;
        borderPath.addEllipse (wellBounds.reduced (0.5f));

        juce::Path outline;
        juce::PathStrokeType (1.0f, juce::PathStrokeType::curved, juce::PathStrokeType::butt)
            .createStrokedPath (outline, borderPath);

        juce::ColourGradient borderGrad (
            juce::Colours::black.withAlpha (0.85f),
            centreX, wellBounds.getY(),
            juce::Colours::white.withAlpha (0.20f),
            centreX, wellBounds.getBottom(),
            false);
        g.setGradientFill (borderGrad);
        g.fillPath (outline);
    }

    // ---- Red fill arc ----
    juce::Path fillArc;
    fillArc.addCentredArc (centreX, centreY, arcRadius, arcRadius,
                           0.0f, rotaryStartAngle, angle, true);

// ---- Hover glow (scaled by effectiveAlpha) ----
    if (effectiveAlpha > 0.01f)
    {
        const auto glowRed = Theme::accentRedBright;

        struct GlowLayer { float extraThickness; float alpha; };
        const GlowLayer layers[] = {
            { 8.0f, 0.07f },
            {  6.0f, 0.08f },
            {  4.5f, 0.14f },
            {  2.5f, 0.24f },
            {  1.0f, 0.35f }
        };

        for (auto& layer : layers)
        {
            juce::PathStrokeType glowStroke (arcThickness + layer.extraThickness,
                                              juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded);
            g.setColour (glowRed.withAlpha (layer.alpha * effectiveAlpha));
            g.strokePath (fillArc, glowStroke);
        }
    }

    // ---- Fill arc (blended between idle and saturated hover red) ----
    {
        const auto idleRed  = juce::Colour (0xff8a3535);
        const auto hoverRed = Theme::accentRedBright;
        const auto baseRed  = idleRed.interpolatedWith (hoverRed, effectiveAlpha);

        juce::Graphics::ScopedSaveState save (g);
        juce::PathStrokeType stroke (arcThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);
        juce::Path strokedArc;
        stroke.createStrokedPath (strokedArc, fillArc);
        g.reduceClipRegion (strokedArc, {});

        juce::ColourGradient grad (baseRed.interpolatedWith (juce::Colours::white, 0.02f * effectiveAlpha),
                                    centreX, centreY - radius,
                                    baseRed.darker (0.22f),
                                    centreX, centreY + radius, false);
        g.setGradientFill (grad);
        g.fillAll();
    }

// ---- Knob body (flat dark grey cap with top-to-bottom grey border gradient & soft shadow) ----
    float knobRadius = arcRadius - arcThickness * 0.5f - 2.0f;
    juce::Rectangle<float> knobBounds (centreX - knobRadius, centreY - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f);

    // Soft drop shadow below the knob cap
    {
        juce::Path shadowPath;
        shadowPath.addEllipse (knobBounds.translated (0.0f, 2.5f));
        juce::DropShadow softShadow (juce::Colours::black.withAlpha (0.75f), 7, { 0, 1 });
        softShadow.drawForPath (g, shadowPath);
    }

    // Flat dark grey cap
    g.setColour (Theme::surfaceCard.darker (0.25f));
    g.fillEllipse (knobBounds);

    // Full-circle border highlight: light grey/white at top fading into medium-dark grey at bottom
    {
        auto rimBounds = knobBounds.reduced (0.5f);
        juce::Path rimPath;
        rimPath.addEllipse (rimBounds);

        juce::Path strokedRim;
        juce::PathStrokeType (1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::butt)
            .createStrokedPath (strokedRim, rimPath);

        juce::ColourGradient rimGrad (
            juce::Colours::white.withAlpha (0.44f),
            centreX, rimBounds.getY(),
            Theme::surfaceCard.brighter (0.04f),
            centreX, rimBounds.getBottom(),
            false);
        rimGrad.addColour (0.45, Theme::surfaceCard.brighter (0.22f));
        rimGrad.addColour (0.75, Theme::surfaceCard.brighter (0.12f));

        g.setGradientFill (rimGrad);
        g.fillPath (strokedRim);
    }

    // ---- Red thumb line ----
    {
        const float outerR = knobRadius - 3.0f;
        const float innerR = knobRadius * 0.42f;

        const float sinA = std::sin (angle);
        const float cosA = std::cos (angle);

        const float startX = centreX + innerR * sinA;
        const float startY = centreY - innerR * cosA;
        const float endX   = centreX + outerR * sinA;
        const float endY   = centreY - outerR * cosA;

        juce::Path thumbLine;
        thumbLine.startNewSubPath (startX, startY);
        thumbLine.lineTo (endX, endY);

        g.setColour (Theme::accentRedBright);
        g.strokePath (thumbLine, juce::PathStrokeType (2.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
}   

void SvarogLookAndFeel::drawLinearSlider (juce::Graphics& g,
                                         int x, int y, int width, int height,
                                         float sliderPos,
                                         float /*minSliderPos*/, float /*maxSliderPos*/,
                                         juce::Slider::SliderStyle style,
                                         juce::Slider& slider)
{
    if (style != juce::Slider::LinearHorizontal && style != juce::Slider::LinearBar)
    {
        juce::LookAndFeel_V4::drawLinearSlider (g, x, y, width, height, sliderPos,
                                                 0.0f, 0.0f, style, slider);
        return;
    }

    auto bounds = slider.getLocalBounds().toFloat();
    bounds = bounds.withSizeKeepingCentre (bounds.getWidth(), 16.0f);
    bounds = bounds.reduced (6.0f, 0.0f);

    const float hoverAlpha = juce::jlimit (0.0f, 1.0f,
        (float) slider.getProperties().getWithDefault ("hoverAlpha", 0.0f));

    const bool isDragging = slider.isMouseButtonDown();
    const float effectiveAlpha = isDragging ? 1.0f : hoverAlpha;

    const float cornerRadius = Radius::control;
    const float minPos = bounds.getX();
    const float maxPos = bounds.getRight();
    const float fillRight = juce::jlimit (minPos, maxPos, sliderPos);

    // ---------- Circle-thumb style (Pan slider) ----------
    if (slider.getProperties().getWithDefault ("circleThumb", false))
    {
        auto wellBounds = bounds;

        // 1. Well background
        {
            const auto fill = Theme::surfaceRaised.darker (0.35f);
            g.setColour (fill);
            g.fillRoundedRectangle (wellBounds, cornerRadius);

            juce::Graphics::ScopedSaveState save (g);
            juce::Path clip;
            clip.addRoundedRectangle (wellBounds, cornerRadius);
            g.reduceClipRegion (clip, {});

            const float shadowHeight = 8.0f;
            juce::ColourGradient topShadow (
                juce::Colours::black.withAlpha (0.0f), wellBounds.getX(), wellBounds.getY(),
                juce::Colours::black.withAlpha (0.0f), wellBounds.getX(), wellBounds.getY() + shadowHeight, false);
            topShadow.addColour (0.15, juce::Colours::black.withAlpha (0.55f));
            topShadow.addColour (0.40, juce::Colours::black.withAlpha (0.30f));
            topShadow.addColour (0.70, juce::Colours::black.withAlpha (0.12f));
            g.setGradientFill (topShadow);
            g.fillRect (wellBounds.withHeight (shadowHeight + 1.0f));
        }

        // 2. Circle thumb
        {
            const auto idleRed  = juce::Colour (0xff8a3535);
            const auto hoverRed = Theme::accentRedBright;
            const auto baseRed  = idleRed.interpolatedWith (hoverRed, effectiveAlpha);

            float cx = juce::jmap (juce::jlimit (0.0f, 1.0f, (fillRight - wellBounds.getX()) / wellBounds.getWidth()),
                                    0.0f, 1.0f,
                                    wellBounds.getX() + 8.0f, wellBounds.getRight() - 8.0f);
            float cy = wellBounds.getCentreY();
            float cr = wellBounds.getHeight() * 0.42f;

            // Glow (drawn behind the circle) — fades with effectiveAlpha
            if (effectiveAlpha > 0.01f)
            {
                juce::Path glowPath;
                glowPath.addEllipse (cx - cr - 2.0f, cy - cr - 2.0f, (cr + 2.0f) * 2.0f, (cr + 2.0f) * 2.0f);
                juce::DropShadow glow (Theme::accentRedBright.withAlpha (0.72f * effectiveAlpha), 10, { 0, 0 });
                glow.drawForPath (g, glowPath);
            }

            // Inner highlight
            if (effectiveAlpha > 0.01f)
            {
                juce::Path brightPath;
                brightPath.addEllipse (cx - cr + 2.0f, cy - cr + 2.0f, (cr - 2.0f) * 2.0f, (cr - 2.0f) * 2.0f);
                juce::DropShadow inner (juce::Colours::white.withAlpha (0.25f * effectiveAlpha), 4, { 0, 0 });
                inner.drawForPath (g, brightPath);
            }

// Optional subtle top highlight on the circle (stays rich red, doesn't wash out)
            if (effectiveAlpha > 0.01f)
            {
                juce::Path brightPath;
                brightPath.addEllipse (cx - cr + 2.0f, cy - cr + 2.0f, (cr - 2.0f) * 2.0f, (cr - 2.0f) * 2.0f);
                juce::DropShadow inner (juce::Colours::white.withAlpha (0.04f * effectiveAlpha), 3, { 0, -1 });
                inner.drawForPath (g, brightPath);
            }

            // Filled circle
            juce::Path circlePath;
            circlePath.addEllipse (cx - cr, cy - cr, cr * 2.0f, cr * 2.0f);

            const auto topCircleRed = baseRed.interpolatedWith (juce::Colours::white, 0.08f + 0.06f * effectiveAlpha);
            juce::ColourGradient circleGrad (
                topCircleRed, cx, cy - cr,
                baseRed.darker (0.24f), cx, cy + cr,
                false);
            g.setGradientFill (circleGrad);
            g.fillPath (circlePath);

            // Circle border
            g.setColour (juce::Colours::black.withAlpha (0.6f));
            g.drawEllipse (cx - cr, cy - cr, cr * 2.0f, cr * 2.0f, 1.0f);
        }

        // 3. Border
        {
            juce::Path borderPath;
            borderPath.addRoundedRectangle (wellBounds.reduced (0.5f),
                                            juce::jmax (0.0f, cornerRadius - 0.5f));
            juce::Path outline;
            juce::PathStrokeType (1.0f, juce::PathStrokeType::curved, juce::PathStrokeType::butt)
                .createStrokedPath (outline, borderPath);
            juce::ColourGradient borderGrad (
                juce::Colours::black.withAlpha (0.8f), wellBounds.getX(), wellBounds.getY(),
                juce::Colours::white.withAlpha (0.2f), wellBounds.getX(), wellBounds.getBottom(), false);
            borderGrad.addColour (0.7, juce::Colours::grey.withAlpha (0.25f));
            g.setGradientFill (borderGrad);
            g.fillPath (outline);
        }

        return;
    }

    // ---------- 1. Well background ----------
    {
        const auto fill = Theme::surfaceRaised.darker (0.35f);
        g.setColour (fill);
        g.fillRoundedRectangle (bounds, cornerRadius);

        juce::Graphics::ScopedSaveState save (g);
        juce::Path clip;
        clip.addRoundedRectangle (bounds, cornerRadius);
        g.reduceClipRegion (clip, {});

        const float shadowHeight = 8.0f;
        juce::ColourGradient topShadow (
            juce::Colours::black.withAlpha (0.0f), bounds.getX(), bounds.getY(),
            juce::Colours::black.withAlpha (0.0f), bounds.getX(), bounds.getY() + shadowHeight, false);
        topShadow.addColour (0.15, juce::Colours::black.withAlpha (0.55f));
        topShadow.addColour (0.40, juce::Colours::black.withAlpha (0.30f));
        topShadow.addColour (0.70, juce::Colours::black.withAlpha (0.12f));
        g.setGradientFill (topShadow);
        g.fillRect (bounds.withHeight (shadowHeight + 1.0f));
    }

    // ---------- 2. Red fill (clipped to the well) ----------
    if (fillRight > minPos + 0.5f)
    {
        juce::Graphics::ScopedSaveState save (g);
        auto fillBounds = bounds.reduced (1.5f, 1.5f);
        float fillWidth = fillRight - fillBounds.getX();
        if (fillWidth < 1.0f) fillWidth = 1.0f;

        const float innerRadius = juce::jmax (0.0f, cornerRadius - 1.5f);
        juce::Path fillClip;
        fillClip.addRoundedRectangle (fillBounds.getX(), fillBounds.getY(), fillWidth,
                                       fillBounds.getHeight(), innerRadius, innerRadius,
                                       true, true, true, true);
        g.reduceClipRegion (fillClip, {});

        const auto idleRed  = juce::Colour (0xff8a3535);
        const auto hoverRed = Theme::accentRedBright;
        const auto baseRed  = idleRed.interpolatedWith (hoverRed, effectiveAlpha);

        const auto topRed    = baseRed.interpolatedWith (juce::Colours::white, 0.08f + 0.08f * effectiveAlpha);
        const auto bottomRed = baseRed.darker (0.24f);
        juce::ColourGradient grad (topRed, bounds.getX(), bounds.getY(),
                                    bottomRed, bounds.getX(), bounds.getBottom(), false);
        g.setGradientFill (grad);
        g.fillRect (bounds);

        // Subtle top highlight along the top edge of the fill
        {
            const float topSheenAlpha = 0.10f + 0.14f * effectiveAlpha;
            juce::ColourGradient sheen (juce::Colours::white.withAlpha (topSheenAlpha),
                                        bounds.getX(), bounds.getY(),
                                        juce::Colours::transparentWhite,
                                        bounds.getX(), bounds.getY() + 3.5f, false);
            g.setGradientFill (sheen);
            g.fillRect (bounds.withHeight (4.0f));
        }
    }

    // ---------- 3. Glow around the fill ----------
    if (effectiveAlpha > 0.01f && fillRight > minPos + 1.0f)
    {
        auto fillBounds = bounds.reduced (1.5f, 1.5f);
        float fillWidth = fillRight - fillBounds.getX();
        if (fillWidth > 1.0f)
        {
            const float innerRadius = juce::jmax (0.0f, cornerRadius - 1.5f);
            juce::Path fillGlowPath;
            fillGlowPath.addRoundedRectangle (fillBounds.getX(), fillBounds.getY(),
                                               fillWidth, fillBounds.getHeight(),
                                               innerRadius, innerRadius,
                                               true, true, true, true);

juce::DropShadow glow (Theme::accentRedBright.withAlpha (0.95f * effectiveAlpha), 12, { 0, 0 });
            glow.drawForPath (g, fillGlowPath);
        }
    }

    // ---------- 4. Value indicator line ----------
    {
        g.setColour (Theme::background.withAlpha (0.7f));
        g.drawLine (fillRight, bounds.getY() + 2.0f,
                    fillRight, bounds.getBottom() - 2.0f, 1.0f);
    }

    // ---------- 5. Border ----------
    {
        juce::Path borderPath;
        borderPath.addRoundedRectangle (bounds.reduced (0.5f),
                                        juce::jmax (0.0f, cornerRadius - 0.5f));
        juce::Path outline;
        juce::PathStrokeType (1.0f, juce::PathStrokeType::curved, juce::PathStrokeType::butt)
            .createStrokedPath (outline, borderPath);
        juce::ColourGradient borderGrad (
            juce::Colours::black.withAlpha (0.8f), bounds.getX(), bounds.getY(),
            juce::Colours::white.withAlpha (0.2f), bounds.getX(), bounds.getBottom(), false);
        borderGrad.addColour (0.7, juce::Colours::grey.withAlpha (0.25f));
        g.setGradientFill (borderGrad);
        g.fillPath (outline);
    }

    slider.getProperties().set ("wellX", bounds.getX());
    slider.getProperties().set ("wellY", bounds.getY());
    slider.getProperties().set ("wellH", bounds.getHeight());
    slider.getProperties().set ("fillRightX", fillRight);
}

void SvarogLookAndFeel::drawButtonBackground (juce::Graphics& g,
                                             juce::Button& button,
                                             const juce::Colour& /*backgroundColour*/,
                                             bool shouldDrawButtonAsHighlighted,
                                             bool shouldDrawButtonAsDown)
{
    const auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);
    const bool isEnabled = button.isEnabled();
    const bool isOn      = button.getToggleState();
    const bool isDown    = shouldDrawButtonAsDown;

    auto offColour = button.findColour (juce::TextButton::buttonColourId);
    auto onColour  = button.findColour (juce::TextButton::buttonOnColourId);

    if (offColour.isTransparent()) offColour = Theme::surfaceRaised;
    if (onColour.isTransparent())  onColour  = Theme::accentRedBright;

    

    const bool isPrimary = ! isOn && offColour.getARGB() == Theme::accentRed.getARGB();
    juce::Colour base = (isOn || isPrimary) ? onColour : offColour;
    if (! isEnabled) base = base.withMultipliedBrightness (0.55f);

    const auto cornerRadiusVar = button.getProperties()["cornerRadius"];
    const float cornerRadius = cornerRadiusVar.isVoid() ? Radius::control : (float) cornerRadiusVar;

    // Draw glows and shadows!
    if (isEnabled)
    {
        if (isOn)
        {
            // Engaged button: red glow
            drawActiveControlShadow (g, bounds);
            drawRaisedControl (g, bounds, base, shouldDrawButtonAsHighlighted, isDown, false, cornerRadius);
        }
        else
        {
            // Disengaged button: standard shadow
            drawControlShadow (g, bounds);
            drawRaisedControl (g, bounds, base, shouldDrawButtonAsHighlighted, isDown, false, cornerRadius);
        }
    }
    else
    {
        drawRecessedControl (g, bounds, base.darker (0.15f), cornerRadius);
    }
}