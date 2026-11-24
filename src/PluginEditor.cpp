#include "PluginEditor.h"

// =========================================================
//  StereoGraph
// =========================================================
StereoGraph::StereoGraph (SvarogWiderAudioProcessor& p) : audioProcessor(p)
{
    setInterceptsMouseClicks (false, false);
}

void StereoGraph::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    auto* lf = dynamic_cast<SvarogLookAndFeel*> (&getLookAndFeel());
    if (lf) lf->drawWell (g, bounds, Theme::surfaceDeepest);

    // Reserve space on the left for the dB ruler
    auto rulerArea = bounds.removeFromLeft (32.0f);
    auto inner     = bounds.reduced (3.0f);

    // Bottom-centre of the graph is the mono axis
    auto centre = juce::Point<float> (inner.getCentreX(), inner.getBottom());
    float size  = inner.getWidth() * 0.7f;

        // Padded vertical range matching the dB ruler's pixel range
    float plotTop    = inner.getY() + 12.0f;
    float plotBottom = inner.getBottom() - 12.0f;

    // ---- 1. Reference grid (plain 1px lines) ----
    {
        // Faint dB grid — horizontal lines at every 6 dB
        {
            const float topDb    = 12.0f;
            const float bottomDb = -24.0f;
            const float dbTicks[] = { 12.0f, 6.0f, 0.0f, -6.0f, -12.0f, -18.0f, -24.0f };

            // Use a dimmer alpha than the diagonals so the grid sits
            // behind the reference lines visually
            g.setColour (Theme::greyDim.withAlpha (0.12f));

            for (float db : dbTicks)
            {
                float norm = juce::jmap (db, bottomDb, topDb, 0.0f, 1.0f);
                float y    = juce::jmap (norm, 0.0f, 1.0f,
                                          inner.getBottom() - 12.0f,
                                          inner.getY() + 12.0f);
                g.drawLine (inner.getX(), y, inner.getRight(), y, 1.0f);
            }
        }

        // Primary reference lines
        g.setColour (Theme::greyDim.withAlpha (0.35f));

        // Vertical centre line (mono axis) — inset to match the ruler
        g.drawLine (centre.x, plotTop, centre.x, plotBottom, 1.0f);

        // 45° diagonals from bottom-centre to top-left and top-right.
        // Inset by 12px to match the dB ruler's pixel range.
        float plotTop    = inner.getY() + 12.0f;
        float plotBottom = inner.getBottom() - 12.0f;

        // Extend lines 6px past inner.getX() / inner.getRight() so they
        // reach the edge of the dot circles (radius = 6) at max spread.
        g.drawLine (centre.x, plotBottom, inner.getX() - 6.0f, plotTop, 1.0f);
        g.drawLine (centre.x, plotBottom, inner.getRight() + 6.0f, plotTop, 1.0f);
    }

    // ---- 2. dB ruler (left side) ----
    {
        const float topDb    = 12.0f;
        const float bottomDb = -24.0f;

        g.setColour (Theme::textFaint);
        g.setFont (juce::Font (juce::FontOptions (SvarogLookAndFeel::getInterRegular()).withHeight (9.0f)));

        // Ticks every 6 dB
        const float dbTicks[] = { 12.0f, 6.0f, 0.0f, -6.0f, -12.0f, -18.0f, -24.0f };

        for (float db : dbTicks)
        {
            float norm   = juce::jmap (db, bottomDb, topDb, 0.0f, 1.0f);
            float y      = juce::jmap (norm, 0.0f, 1.0f,
                                        inner.getBottom() - 12.0f,
                                        inner.getY() + 12.0f);

            // Tick mark
            g.setColour (Theme::greyDim.withAlpha (0.5f));
            g.drawLine (rulerArea.getRight() - 6.0f, y,
                        rulerArea.getRight(), y, 1.0f);

            // Label
            g.setColour (Theme::textFaint);
            juce::String label = (db > 0 ? "+" : "") + juce::String ((int) db);
            g.drawText (label,
                        juce::Rectangle<float> (rulerArea.getX() + 2.0f, y - 6.0f,
                                                 rulerArea.getWidth() - 8.0f, 12.0f),
                        juce::Justification::centredRight, false);
        }
    }

    // ---- 3. Goniometer hazy cloud ----
    {
        const int bufSize = SvarogWiderAudioProcessor::getGonioSize();
        const int wp = audioProcessor.getGonioWritePos();

        struct PointData { float x, y, sx, sy, dist; };
        std::vector<PointData> points;
        points.reserve (bufSize);

        float density = 0.0f;

        for (int i = 0; i < bufSize; ++i)
        {
            int idx = (wp + i) & (bufSize - 1);
            float l = audioProcessor.getGonioL (idx);
            float r = audioProcessor.getGonioR (idx);

            float x = (r - l) * 0.7071f;  
            float y = (l + r) * 0.7071f;

            float mag = std::abs (x) + std::abs (y);
            if (mag < 0.001f) continue;

            float px = centre.x + x * size;
            float py = centre.y - y * size;

            if (inner.contains (px, py))
            {
                float dist = std::sqrt (x * x + y * y);
                points.push_back ({ x, y, px, py, dist });
            }
        }

        density = juce::jlimit (0.0f, 1.0f, (float) points.size() / (float) bufSize);

        if (! points.empty() && density > 0.01f)
        {
            // --- 3a. Radial bloom from mono axis ---
            {
                juce::ColourGradient bloom (
                    Theme::accentRedBright.withAlpha (0.35f * density),
                    centre.x, centre.y,
                    juce::Colours::transparentBlack,
                    centre.x, centre.y - inner.getWidth() * 0.5f,
                    false);
                bloom.addColour (0.4f, Theme::accentRedBright.withAlpha (0.18f * density));
                bloom.addColour (0.75f, Theme::accentRedBright.withAlpha (0.06f * density));
                bloom.addColour (1.0f, juce::Colours::transparentBlack);

                g.setGradientFill (bloom);
                g.fillRect (inner);
            }

            // --- 3b. Widest outer haze ---
            g.setColour (Theme::accentRedBright.withAlpha (0.02f * (0.5f + density)));
            for (auto& p : points)
                g.fillEllipse (p.sx - 4.0f, p.sy - 4.0f, 8.0f, 8.0f);

            // --- 3c. Medium glow ring ---
            g.setColour (Theme::accentRedBright.withAlpha (0.05f * (0.5f + density)));
            for (auto& p : points)
                g.fillEllipse (p.sx - 2.2f, p.sy - 2.2f, 4.4f, 4.4f);

            // --- 3d. Main cloud ---
            for (auto& p : points)
            {
                float falloff = juce::jlimit (0.0f, 1.0f, 1.0f - p.dist * 1.15f);
                falloff = std::pow (falloff, 0.65f);
                float alpha = falloff * (0.55f + density * 0.35f);

                g.setColour (Theme::accentRedBright.withAlpha (alpha));
                g.fillEllipse (p.sx - 0.75f, p.sy - 0.75f, 1.5f, 1.5f);
            }

            // --- 3e. White core ---
            g.setColour (juce::Colours::white.withAlpha (0.15f + density * 0.35f));
            for (auto& p : points)
            {
                if (p.dist < 0.08f)
                    g.fillEllipse (p.sx - 0.6f, p.sy - 0.6f, 1.2f, 1.2f);
            }
        }
    }

    // ---- 4. Voice dots along the 45° lines ----
    // The dot's path goes from M (top centre) to L or R (bottom corner).
    // Volume controls how far along the line the dot sits.
    auto drawDot = [&] (float pan, float volume, float pulseAlpha, juce::Colour color)
    {
        // The dot maps to a point that lies on either the left or right
        // 45° diagonal, starting from M at the top of the graph.
        //
        // t = 0   → at M (top centre)
        // t = 1   → at the bottom left/right corner (near L or R)
        //
        // Volume = 0dB  →  t = 0   (dot near M)
        // Volume = -24dB →  t = 1   (dot at bottom corner)

        float volNorm = juce::jlimit (0.0f, 1.0f,
                                       juce::jmap (volume, -24.0f, 12.0f, 0.0f, 1.0f));
        // At +12dB, clamp to top (t=0). At -24dB, bottom (t=1).

        float t = volNorm;

        // Pan determines which side of M the dot goes toward.
        // pan = -100 → full L side; pan = +100 → full R side;
        // pan = 0    → straight down the centre axis.
        float panNorm = juce::jlimit (-1.0f, 1.0f, pan / 100.0f);

        // Horizontal extent from M to L/R corner
        float spread = inner.getWidth() * 0.5f;

        // Y uses the ruler's padded range so the dot's vertical position
        // matches the grid lines and dB labels exactly.
        float plotTop    = inner.getY() + 12.0f;
        float plotBottom = inner.getBottom() - 12.0f;

        float x = centre.x + panNorm * spread * t;
        float y = juce::jmap (t, 0.0f, 1.0f, plotBottom, plotTop);

        float radius = 6.0f;

         if (pulseAlpha > 0.0f)
        {
            // Layer 1: innermost, tight core (very bright)
            {
                const float r = radius * 1.6f;
                juce::ColourGradient grad1 (color.withAlpha (juce::jmin (1.0f, 1.0f * pulseAlpha)),
                                             x, y,
                                             juce::Colours::transparentBlack,
                                             x + r, y,
                                             true);
                g.setGradientFill (grad1);
                g.fillEllipse (x - r, y - r, r * 2.0f, r * 2.0f);
            }

            // Layer 2: medium halo (bright)
            {
                const float r = radius * 3.0f;
                juce::ColourGradient grad2 (color.withAlpha (juce::jmin (1.0f, 0.75f * pulseAlpha)),
                                             x, y,
                                             juce::Colours::transparentBlack,
                                             x + r, y,
                                             true);
                grad2.addColour (0.3f, color.withAlpha (0.45f * pulseAlpha));
                grad2.addColour (0.7f, color.withAlpha (0.15f * pulseAlpha));
                g.setGradientFill (grad2);
                g.fillEllipse (x - r, y - r, r * 2.0f, r * 2.0f);
            }

            // Layer 3: outer haze (faint but wide)
            {
                const float r = radius * 4.0f;
                juce::ColourGradient grad3 (color.withAlpha (juce::jmin (1.0f, 0.40f * pulseAlpha)),
                                             x, y,
                                             juce::Colours::transparentBlack,
                                             x + r, y,
                                             true);
                grad3.addColour (0.4f, color.withAlpha (0.18f * pulseAlpha));
                grad3.addColour (0.8f, color.withAlpha (0.05f * pulseAlpha));
                g.setGradientFill (grad3);
                g.fillEllipse (x - r, y - r, r * 2.0f, r * 2.0f);
            }
        }

        g.setColour (color);
        g.fillEllipse (x - radius, y - radius, radius * 2.0f, radius * 2.0f);
        g.setColour (juce::Colours::white.withAlpha (0.8f));
        g.fillEllipse (x - 1.5f, y - 1.5f, 3.0f, 3.0f);
    };

    bool origOn = audioProcessor.apvts.getRawParameterValue (SvarogWiderAudioProcessor::originalOnId())->load() > 0.5f;
    float origDb = audioProcessor.apvts.getRawParameterValue (SvarogWiderAudioProcessor::originalVolumeId())->load();
    if (origOn) drawDot (0.0f, origDb, 0.0f, Theme::textPrimary);

    juce::Colour voiceColors[4] = {
        juce::Colour (0xffe04040),   // Red
        juce::Colour (0xff40c040),   // Green
        juce::Colour (0xffd0c040),   // Yellow
        juce::Colour (0xff4080e0)    // Blue
    };

    for (int i = 0; i < SvarogWiderAudioProcessor::kNumCopies; ++i)
    {
        int id = i + 1;
        bool on = audioProcessor.apvts.getRawParameterValue (SvarogWiderAudioProcessor::copyOnId (id))->load() > 0.5f;
        if (!on) continue;

        float pan = audioProcessor.apvts.getRawParameterValue (SvarogWiderAudioProcessor::copyPanId (id))->load();
        float volDb = audioProcessor.apvts.getRawParameterValue (SvarogWiderAudioProcessor::copyVolumeId (id))->load();
        float depth = audioProcessor.apvts.getRawParameterValue (SvarogWiderAudioProcessor::copyModDepthId (id))->load();

        // Zero-depth safety: no pulse when depth is 0
        float lfo = std::sin (audioProcessor.getCopyLfoValue (i));
        float lfoNorm = 0.5f + 0.5f * lfo;    // 0 at trough, 1 at peak
        float depthScale = std::abs (depth) / 100.0f;

        // Fade in the pulse smoothly as depth goes above 0.
        // At depth=0: pulseAlpha = 0 (no glow).
        // At depth=10%: pulseAlpha ≈ 0.4–0.6 (visible pulse).
        // At depth=100%: pulseAlpha ≈ 0.9–1.0 (full glow).
        float base = juce::jmin (1.0f, depthScale * 5.0f);   // ramps 0→1 as depth goes 0→33%
        float pulseAlpha = base * (0.55f + 0.45f * lfoNorm);

        drawDot (pan, volDb, pulseAlpha, voiceColors[i]);
    }

    // ---- 5. Labels ----
    g.setColour (Theme::textFaint);
    g.setFont (juce::Font (juce::FontOptions (SvarogLookAndFeel::getInterBold()).withHeight (9.0f)));

    g.drawText ("L", juce::Rectangle<float> (inner.getX() + 6.0f, plotBottom - 14.0f, 12.0f, 12.0f),
                juce::Justification::centred, false);
    g.drawText ("R", juce::Rectangle<float> (inner.getRight() - 18.0f, plotBottom - 14.0f, 12.0f, 12.0f),
                juce::Justification::centred, false);
    g.drawText ("M", juce::Rectangle<float> (centre.x - 6.0f, plotTop - 14.0f, 12.0f, 12.0f),
                juce::Justification::centred, false);
}

// =========================================================
//  LfoGraph
// =========================================================
LfoGraph::LfoGraph (SvarogWiderAudioProcessor& p, int idx) : audioProcessor(p), copyIndex(idx)
{
    setInterceptsMouseClicks (false, false);
}

void LfoGraph::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto* lf = dynamic_cast<SvarogLookAndFeel*> (&getLookAndFeel());
    if (lf) lf->drawWell (g, bounds, Theme::surfaceDeepest);

    int id = copyIndex + 1;
    bool on = audioProcessor.apvts.getRawParameterValue (SvarogWiderAudioProcessor::copyOnId (id))->load() > 0.5f;
    if (!on) return;

    auto plotArea = bounds.reduced (2.0f, 4.0f);

    // --- Radial gradient background ---
    {
        juce::Graphics::ScopedSaveState save (g);

        juce::Path clip;
        clip.addRoundedRectangle (bounds.reduced (0.5f), Radius::well);
        g.reduceClipRegion (clip, {});

        auto centre = bounds.getCentre();
        float radius = bounds.getWidth() * 0.6f;

        juce::ColourGradient bgGrad (
            juce::Colour (0xff4a1a1a),
            centre.x, centre.y,
            juce::Colours::transparentBlack,
            centre.x + radius, centre.y,
            true);
        bgGrad.addColour (0.30f, juce::Colour (0xff381414));
        bgGrad.addColour (0.60f, juce::Colour (0xff200808));
        bgGrad.addColour (0.85f, juce::Colours::transparentBlack);
        bgGrad.addColour (1.00f, juce::Colours::transparentBlack);

        g.setGradientFill (bgGrad);
        g.fillRect (bounds);
    }

    // --- Read parameters ---
    float speed = audioProcessor.apvts.getRawParameterValue (SvarogWiderAudioProcessor::copyModSpeedId (id))->load();
    float depth = audioProcessor.apvts.getRawParameterValue (SvarogWiderAudioProcessor::copyModDepthId (id))->load() / 100.0f;
    float currentPhase = audioProcessor.getCopyLfoValue (copyIndex);

    float midY = plotArea.getCentreY();
    float amp  = (plotArea.getHeight() * 0.4f) * depth;

    // Build the live wave path
    juce::Path p;
    for (int x = 0; x < (int) plotArea.getWidth(); ++x)
    {
        float normalisedX = (float) x / plotArea.getWidth();
        float wavePhase = (normalisedX * juce::MathConstants<float>::twoPi * 2.0f) - juce::MathConstants<float>::pi;
        float y = midY + std::sin (wavePhase * speed + currentPhase) * amp;
        float px = plotArea.getX() + (float) x;

        if (x == 0) p.startNewSubPath (px, y);
        else        p.lineTo (px, y);
    }

    // ---- Wave (clipped to plot area) ----
    {
        juce::Graphics::ScopedSaveState save (g);
        g.reduceClipRegion (plotArea.toNearestInt());

        g.setColour (Theme::accentRedBright.withAlpha (0.85f));
        g.strokePath (p, juce::PathStrokeType (2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // --- Inner shadow: soft fade top, bottom, and sides ---
    {
        juce::Graphics::ScopedSaveState save (g);
        juce::Path clip;
        clip.addRoundedRectangle (bounds, Radius::well);
        g.reduceClipRegion (clip, {});

        const float topFade    = 20.0f;
        const float bottomFade = 22.0f;
        const float sideFade   = 14.0f;

        // Top shadow — gradual fade
        {
            juce::ColourGradient topShadow (
                juce::Colours::black.withAlpha (0.55f),
                bounds.getX(), bounds.getY(),
                juce::Colours::transparentBlack,
                bounds.getX(), bounds.getY() + topFade,
                false);
            topShadow.addColour (0.25f, juce::Colours::black.withAlpha (0.40f));
            topShadow.addColour (0.55f, juce::Colours::black.withAlpha (0.20f));
            topShadow.addColour (0.80f, juce::Colours::black.withAlpha (0.06f));
            g.setGradientFill (topShadow);
            g.fillRect (bounds.withHeight (topFade + 1.0f));
        }

        // Bottom shadow — gradual fade
        {
            juce::ColourGradient bottomShadow (
                juce::Colours::black.withAlpha (0.45f),
                bounds.getX(), bounds.getBottom(),
                juce::Colours::transparentBlack,
                bounds.getX(), bounds.getBottom() - bottomFade,
                false);
            bottomShadow.addColour (0.25f, juce::Colours::black.withAlpha (0.32f));
            bottomShadow.addColour (0.55f, juce::Colours::black.withAlpha (0.16f));
            bottomShadow.addColour (0.80f, juce::Colours::black.withAlpha (0.05f));
            g.setGradientFill (bottomShadow);
            g.fillRect (juce::Rectangle<float> (bounds.getX(),
                                                bounds.getBottom() - bottomFade,
                                                bounds.getWidth(),
                                                bottomFade));
        }

        // Left shadow — fade from left edge rightward
        {
            juce::ColourGradient leftShadow (
                juce::Colours::black.withAlpha (0.60f),
                bounds.getX(), bounds.getY(),
                juce::Colours::transparentBlack,
                bounds.getX() + sideFade, bounds.getY(),
                false);
            leftShadow.addColour (0.25f, juce::Colours::black.withAlpha (0.40f));
            leftShadow.addColour (0.55f, juce::Colours::black.withAlpha (0.18f));
            leftShadow.addColour (0.80f, juce::Colours::black.withAlpha (0.05f));
            g.setGradientFill (leftShadow);
            g.fillRect (juce::Rectangle<float> (bounds.getX(),
                                                bounds.getY(),
                                                sideFade,
                                                bounds.getHeight()));
        }

        // Right shadow — fade from right edge leftward
        {
            juce::ColourGradient rightShadow (
                juce::Colours::black.withAlpha (0.60f),
                bounds.getRight(), bounds.getY(),
                juce::Colours::transparentBlack,
                bounds.getRight() - sideFade, bounds.getY(),
                false);
            rightShadow.addColour (0.25f, juce::Colours::black.withAlpha (0.40f));
            rightShadow.addColour (0.55f, juce::Colours::black.withAlpha (0.18f));
            rightShadow.addColour (0.80f, juce::Colours::black.withAlpha (0.05f));
            g.setGradientFill (rightShadow);
            g.fillRect (juce::Rectangle<float> (bounds.getRight() - sideFade,
                                                bounds.getY(),
                                                sideFade,
                                                bounds.getHeight()));
        }

        // Bottom lip
        {
            const float lipCornerRadius = 4.0f;

            juce::Path lipPath;
            lipPath.addRoundedRectangle (bounds.reduced (0.7f), lipCornerRadius);

            juce::Path outline;
            juce::PathStrokeType (1.5f,
                                   juce::PathStrokeType::curved,
                                   juce::PathStrokeType::butt)
                .createStrokedPath (outline, lipPath);

            juce::Graphics::ScopedSaveState saveInner (g);
            g.reduceClipRegion (juce::Rectangle<int> (
                (int) bounds.getX(),
                (int) bounds.getBottom() - 3,
                (int) bounds.getWidth(),
                3));

            g.setColour (juce::Colours::white.withAlpha (0.25f));
            g.fillPath (outline);
        }
    }

    // --- Dot at the center (drawn last so shadow doesn't dim it) ---
    float centerX = plotArea.getCentreX();
    float centerPhaseOffset = (0.5f * juce::MathConstants<float>::twoPi * 2.0f) - juce::MathConstants<float>::pi;
    float dotY = midY + std::sin (centerPhaseOffset * speed + currentPhase) * amp;

    g.setColour (Theme::textPrimary);
    g.fillEllipse (centerX - 3.0f, dotY - 3.0f, 6.0f, 6.0f);
}

// =========================================================
//  LowCutGraph
// =========================================================
LowCutGraph::LowCutGraph (SvarogWiderAudioProcessor& p) : audioProcessor(p)
{
    setInterceptsMouseClicks (false, false);
}

void LowCutGraph::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto* lf = dynamic_cast<SvarogLookAndFeel*> (&getLookAndFeel());
    if (lf) lf->drawWell (g, bounds, Theme::surfaceDeepest);

    bool on = audioProcessor.apvts.getRawParameterValue (SvarogWiderAudioProcessor::lowCutOnId())->load() > 0.5f;

    float cutoff = audioProcessor.apvts.getRawParameterValue (SvarogWiderAudioProcessor::lowCutId())->load();
    float normCutoff = std::log10(cutoff / 20.0f) / std::log10(500.0f / 20.0f);
    normCutoff = juce::jlimit (0.0f, 1.0f, normCutoff);

    auto plotArea = bounds.reduced (4.0f, 4.0f);

    const float slopeWidth = 13.0f;

    float kneeStartX = plotArea.getX() + normCutoff * (plotArea.getWidth() - slopeWidth);
    float kneeEndX   = kneeStartX + slopeWidth;

    float plateauY = plotArea.getY() + plotArea.getHeight() * 0.33f;

    // Opacity: full when on, dimmed when off
    const float alpha = on ? 1.0f : 0.25f;

    // --- Fill ---
    juce::Path fillPath;
    fillPath.startNewSubPath (plotArea.getX(), plotArea.getBottom());
    fillPath.lineTo          (kneeStartX, plotArea.getBottom());
    fillPath.lineTo          (kneeEndX, plateauY);
    fillPath.lineTo          (plotArea.getRight(), plateauY);
    fillPath.lineTo          (plotArea.getRight(), plotArea.getBottom());
    fillPath.closeSubPath();

    g.setColour (Theme::accentRedBright.withAlpha (0.20f * alpha));
    g.fillPath (fillPath);

    // --- Stroke ---
    juce::Path strokePath;
    strokePath.startNewSubPath (kneeStartX, plotArea.getBottom());
    strokePath.lineTo          (kneeEndX, plateauY);
    strokePath.lineTo          (plotArea.getRight(), plateauY);

    g.setColour (Theme::accentRedBright.withAlpha (alpha));
    g.strokePath (strokePath, juce::PathStrokeType (2.0f));
}

// =========================================================
//  Main Editor
// =========================================================
SvarogWiderAudioProcessorEditor::SvarogWiderAudioProcessorEditor (SvarogWiderAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), graph (p)
{
    setLookAndFeel (&lookAndFeel);

    addAndMakeVisible (graph);

        // ---- Master output control (in the header bar) ----
    outputLabel.setText ("Output Volume", juce::dontSendNotification);
    outputLabel.setJustificationType (juce::Justification::centredRight);
    outputLabel.setColour (juce::Label::textColourId, Theme::textPrimary);
    outputLabel.setFont (juce::Font (juce::FontOptions (SvarogLookAndFeel::getInterBold()).withHeight (Typography::labelSize + 2.0f)));
    addAndMakeVisible (outputLabel);

    outputValueBox = std::make_unique<DraggableValueBox> (
        audioProcessor.apvts, SvarogWiderAudioProcessor::outputGainId(), "", "dB", 0.0);
    outputValueBox->setShowPrefix (false);
    addAndMakeVisible (*outputValueBox);

    auto styleHeader = [] (juce::Label& l, const juce::String& text)
    {
        l.setText (text, juce::dontSendNotification);
        l.setJustificationType (juce::Justification::centredTop);
        l.setColour (juce::Label::textColourId, Theme::textPrimary); // Lighter white
        l.setFont (juce::Font (juce::FontOptions (SvarogLookAndFeel::getInterBold()).withHeight (Typography::labelSize + 3.0f))); // Larger
    };

    auto styleKnob = [] (juce::Slider& s, juce::Label& l, const juce::String& name)
    {
        s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 18);
        s.setColour (juce::Slider::rotarySliderFillColourId, Theme::accentRedBright);
        s.setColour (juce::Slider::thumbColourId, Theme::textPrimary);
        s.setColour (juce::Slider::textBoxTextColourId, Theme::textPrimary);
        s.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        s.setVelocityBasedMode (false);
        s.setMouseDragSensitivity (300);

        l.setText (name, juce::dontSendNotification);
        l.setJustificationType (juce::Justification::centred);
        l.setColour (juce::Label::textColourId, Theme::textSecondary);
        l.setFont (juce::Font (juce::FontOptions (SvarogLookAndFeel::getInterBold()).withHeight (Typography::labelSize + 2.0f)));
    };

    auto styleSmallLabel = [] (juce::Label& l, const juce::String& text)
    {
        l.setText (text, juce::dontSendNotification);
        l.setJustificationType (juce::Justification::centredLeft);
        l.setColour (juce::Label::textColourId, Theme::textSecondary);
        l.setFont (juce::Font (juce::FontOptions (SvarogLookAndFeel::getInterRegular()).withHeight (Typography::labelSize + 2.0f))); // Slightly larger
    };

    auto styleOnButton = [] (RepaintOnToggleButton& b)
    {
        b.setClickingTogglesState (true);
        b.setButtonText ("ON");
        b.setColour (juce::TextButton::buttonColourId,   Theme::surfaceRaised);
        b.setColour (juce::TextButton::buttonOnColourId, Theme::accentRedBright);
        b.setColour (juce::TextButton::textColourOffId,  Theme::textSecondary);
        b.setColour (juce::TextButton::textColourOnId,   Theme::textPrimary);
        b.getProperties().set ("fontSize", 10.0f);
    };

    // ---------- ORIGINAL ----------
    styleHeader (originalHeaderLabel, "Original");
    styleOnButton (originalOnButton);
    originalOnButton.setPaintingIsUnclipped (true);
    styleKnob (originalVolumeKnob, originalVolumeLabel, "Volume");
    addAndMakeVisible (originalHeaderLabel);
    addAndMakeVisible (originalOnButton);
    addAndMakeVisible (originalVolumeKnob);
    addAndMakeVisible (originalVolumeLabel);

    originalOnAttachment = std::make_unique<ButtonAttachment> (
        audioProcessor.apvts, SvarogWiderAudioProcessor::originalOnId(), originalOnButton);
    originalVolumeAttachment = std::make_unique<SliderAttachment> (
        audioProcessor.apvts, SvarogWiderAudioProcessor::originalVolumeId(), originalVolumeKnob);

    inputMeter = std::make_unique<LevelMeterComponent>();
    inputMeter->setSource (&audioProcessor.getInputMeterSource());
    inputMeter->setLabel ("IN");
    addAndMakeVisible (*inputMeter);

    // ---------- LOW CUT ----------
    styleHeader (lowCutHeaderLabel, "Voices LCF");
    styleOnButton (lowCutOnButton);
    lowCutOnButton.setPaintingIsUnclipped (true);
    addAndMakeVisible (lowCutHeaderLabel);
    addAndMakeVisible (lowCutOnButton);

    lowCutSlider.setSliderStyle (juce::Slider::LinearBar);
    lowCutSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    lowCutSlider.setSliderSnapsToMousePosition (false);
    lowCutSlider.setPaintingIsUnclipped (true); // <-- add this
    addAndMakeVisible (lowCutSlider);
    lowCutSliderAttachment = std::make_unique<SliderAttachment> (
        audioProcessor.apvts, SvarogWiderAudioProcessor::lowCutId(), lowCutSlider);

    lowCutValueBox = std::make_unique<DraggableValueBox> (
        audioProcessor.apvts, SvarogWiderAudioProcessor::lowCutId(), "", "Hz", 100.0);
    lowCutValueBox->setShowPrefix (false);
    addAndMakeVisible (*lowCutValueBox);

    lowCutOnAttachment = std::make_unique<ButtonAttachment> (
        audioProcessor.apvts, SvarogWiderAudioProcessor::lowCutOnId(), lowCutOnButton);

    lowCutGraph = std::make_unique<LowCutGraph> (p);
    addAndMakeVisible (*lowCutGraph);

    outputMeter = std::make_unique<LevelMeterComponent>();
    outputMeter->setSource (&audioProcessor.getOutputMeterSource());
    outputMeter->setLabel ("OUT");
    addAndMakeVisible (*outputMeter);

    // ---------- VOICES ----------
    for (int i = 0; i < SvarogWiderAudioProcessor::kNumCopies; ++i)
    {
        const int id = i + 1;
        auto& cc = copyControls[i];

        // Create the color dot for this voice
        static const juce::Colour voiceDotColors[4] = {
            juce::Colour (0xffe04040),   // Red
            juce::Colour (0xff40c040),   // Green
            juce::Colour (0xffd0c040),   // Yellow
            juce::Colour (0xff4080e0)    // Blue
        };

        cc.voiceDot = std::make_unique<VoiceColorDot> (voiceDotColors[i]);
        addAndMakeVisible (*cc.voiceDot);

        styleHeader (cc.headerLabel, "Voice " + juce::String (id));
        styleOnButton (cc.onButton);
        cc.onButton.setPaintingIsUnclipped (true);
        addAndMakeVisible (cc.headerLabel);
        addAndMakeVisible (cc.onButton);

        cc.onAttachment = std::make_unique<ButtonAttachment> (
            audioProcessor.apvts, SvarogWiderAudioProcessor::copyOnId (id), cc.onButton);

        styleKnob (cc.volumeKnob, cc.volumeLabel, "Volume");
        addAndMakeVisible (cc.volumeKnob);
        addAndMakeVisible (cc.volumeLabel);
        cc.volumeAttachment = std::make_unique<SliderAttachment> (
            audioProcessor.apvts, SvarogWiderAudioProcessor::copyVolumeId (id), cc.volumeKnob);

        // Pan
        styleSmallLabel (cc.panLabel, "Pan");
        addAndMakeVisible (cc.panLabel);

        cc.panSlider.setSliderStyle (juce::Slider::LinearBar);
        cc.panSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        cc.panSlider.setSliderSnapsToMousePosition (false);
        cc.panSlider.getProperties().set ("circleThumb", true); 
        cc.panSlider.setPaintingIsUnclipped (true);
        addAndMakeVisible (cc.panSlider);
        cc.panSliderAttachment = std::make_unique<SliderAttachment> (
            audioProcessor.apvts, SvarogWiderAudioProcessor::copyPanId (id), cc.panSlider);

        cc.panValueBox = std::make_unique<DraggableValueBox> (
            audioProcessor.apvts, SvarogWiderAudioProcessor::copyPanId (id), "", "", 0.0);
        cc.panValueBox->setShowPrefix (false);
        addAndMakeVisible (*cc.panValueBox);

        // Delay
        styleSmallLabel (cc.delayLabel, "Delay");
        addAndMakeVisible (cc.delayLabel);

        cc.delaySlider.setSliderStyle (juce::Slider::LinearBar);
        cc.delaySlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        cc.delaySlider.setSliderSnapsToMousePosition (false);
        cc.delaySlider.setPaintingIsUnclipped (true);
        addAndMakeVisible (cc.delaySlider);
        cc.delaySliderAttachment = std::make_unique<SliderAttachment> (
            audioProcessor.apvts, SvarogWiderAudioProcessor::copyDelayId (id), cc.delaySlider);

        cc.delayValueBox = std::make_unique<DraggableValueBox> (
            audioProcessor.apvts, SvarogWiderAudioProcessor::copyDelayId (id), "", "ms", 15.0);
        cc.delayValueBox->setShowPrefix (false);
        addAndMakeVisible (*cc.delayValueBox);

        // LFO
        cc.lfoGraph = std::make_unique<LfoGraph> (p, i);
        addAndMakeVisible (*cc.lfoGraph);

        // Detune
        styleSmallLabel (cc.detuneLabel, "Detune");
        styleSmallLabel (cc.speedLabel,  "Speed");
        styleSmallLabel (cc.depthLabel,  "Depth");
        addAndMakeVisible (cc.detuneLabel);
        addAndMakeVisible (cc.speedLabel);
        addAndMakeVisible (cc.depthLabel);

        cc.detuneBox = std::make_unique<DraggableValueBox> (
            audioProcessor.apvts, SvarogWiderAudioProcessor::copyDetuneId (id), "", "ct", 0.0);
        cc.detuneBox->setShowPrefix (false);
        addAndMakeVisible (*cc.detuneBox);

        cc.speedBox = std::make_unique<DraggableValueBox> (
            audioProcessor.apvts, SvarogWiderAudioProcessor::copyModSpeedId (id), "", "Hz", 0.5);
        cc.speedBox->setShowPrefix (false);
        addAndMakeVisible (*cc.speedBox);

        cc.depthBox = std::make_unique<DraggableValueBox> (
            audioProcessor.apvts, SvarogWiderAudioProcessor::copyModDepthId (id), "", "%", 30.0);
        cc.depthBox->setShowPrefix (false);
        addAndMakeVisible (*cc.depthBox);
    }

    setResizable (true, true);
    setResizeLimits (680, 500, 2400, 1600);
    setSize (800, 560);

    startTimerHz (30);
}

SvarogWiderAudioProcessorEditor::~SvarogWiderAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void SvarogWiderAudioProcessorEditor::timerCallback()
{
    graph.repaint();
    if (lowCutGraph) lowCutGraph->repaint();
    for (auto& cc : copyControls)
        if (cc.lfoGraph) cc.lfoGraph->repaint();
}

void SvarogWiderAudioProcessorEditor::paint (juce::Graphics& g)
{
    const float scale = getUiScale();

    g.fillAll (Theme::surfaceBase);

    g.setColour (Theme::textPrimary);
    g.setFont (juce::Font (juce::FontOptions (SvarogLookAndFeel::getInterBold())
                                .withHeight (Typography::titleSize * scale)));
    g.drawFittedText ("Svarog Wider",
                      getLocalBounds().removeFromTop (scaled (45)).reduced (scaled (15), 0),
                      juce::Justification::centredLeft, 1);

    auto area = getLocalBounds().reduced (scaled (5));
    area.removeFromTop (scaled (40));

    const float sideFactor = 0.85f;
    const int totalFactor = (int)((4 + 2 * sideFactor) * 100);
    const int baseW = (int)((float)area.getWidth() * 100.0f / (float)totalFactor);
    const int voiceW = baseW;
    const int sideW = (int)((float)baseW * sideFactor);

    juce::Rectangle<int> originalCol = area.removeFromLeft (sideW);
    juce::Rectangle<int> voiceArea   = area.removeFromLeft (voiceW * 4);
    juce::Rectangle<int> lowCutCol   = area.removeFromLeft (sideW);

    const int voicePanelH = juce::jmin (voiceArea.getHeight(), getVoiceColumnContentHeight());

    auto* lf = dynamic_cast<SvarogLookAndFeel*> (&getLookAndFeel());
    if (lf == nullptr) return;

    auto drawColumnPanel = [&] (juce::Rectangle<int> rect)
    {
        auto panel = rect.toFloat().reduced ((float) scaled (2), 0.0f);
        lf->drawPanelShadow (g, panel);
        lf->drawPanel (g, panel, Theme::surfaceRaised);
    };

    drawColumnPanel (originalCol);
    for (int i = 0; i < 4; ++i)
    {
        auto singleVoiceCol = voiceArea.removeFromLeft (voiceW).withHeight (voicePanelH);
        drawColumnPanel (singleVoiceCol);
    }
    drawColumnPanel (lowCutCol);
}

void SvarogWiderAudioProcessorEditor::resized()
{
    const float scale = getUiScale();

    // Update scaled font sizes
    outputLabel.setFont (juce::Font (juce::FontOptions (SvarogLookAndFeel::getInterBold())
                                         .withHeight ((Typography::labelSize + 2.0f) * scale)));

    const float headerFontH = (Typography::labelSize + 3.0f) * scale;
    const float knobLabelH  = (Typography::labelSize + 2.0f) * scale;
    const float smallLabelH = (Typography::labelSize + 2.0f) * scale;

    originalHeaderLabel.setFont (juce::Font (juce::FontOptions (SvarogLookAndFeel::getInterBold()).withHeight (headerFontH)));
    originalVolumeLabel.setFont (juce::Font (juce::FontOptions (SvarogLookAndFeel::getInterBold()).withHeight (knobLabelH)));
    originalOnButton.getProperties().set ("fontSize", 10.0f * scale);

    lowCutHeaderLabel.setFont (juce::Font (juce::FontOptions (SvarogLookAndFeel::getInterBold()).withHeight (headerFontH)));
    lowCutOnButton.getProperties().set ("fontSize", 10.0f * scale);

    for (int i = 0; i < SvarogWiderAudioProcessor::kNumCopies; ++i)
    {
        auto& cc = copyControls[i];
        cc.headerLabel.setFont (juce::Font (juce::FontOptions (SvarogLookAndFeel::getInterBold()).withHeight (headerFontH)));
        cc.volumeLabel.setFont (juce::Font (juce::FontOptions (SvarogLookAndFeel::getInterBold()).withHeight (knobLabelH)));
        cc.panLabel.setFont (juce::Font (juce::FontOptions (SvarogLookAndFeel::getInterRegular()).withHeight (smallLabelH)));
        cc.delayLabel.setFont (juce::Font (juce::FontOptions (SvarogLookAndFeel::getInterRegular()).withHeight (smallLabelH)));
        cc.detuneLabel.setFont (juce::Font (juce::FontOptions (SvarogLookAndFeel::getInterRegular()).withHeight (smallLabelH)));
        cc.speedLabel.setFont (juce::Font (juce::FontOptions (SvarogLookAndFeel::getInterRegular()).withHeight (smallLabelH)));
        cc.depthLabel.setFont (juce::Font (juce::FontOptions (SvarogLookAndFeel::getInterRegular()).withHeight (smallLabelH)));
        cc.onButton.getProperties().set ("fontSize", 10.0f * scale);
    }

    // Scale all DraggableValueBox numbers with the UI
    if (outputValueBox != nullptr) outputValueBox->setUiScale (scale);
    if (lowCutValueBox != nullptr) lowCutValueBox->setUiScale (scale);

    for (int i = 0; i < SvarogWiderAudioProcessor::kNumCopies; ++i)
    {
        auto& cc = copyControls[i];
        if (cc.panValueBox   != nullptr) cc.panValueBox->setUiScale (scale);
        if (cc.delayValueBox != nullptr) cc.delayValueBox->setUiScale (scale);
        if (cc.detuneBox     != nullptr) cc.detuneBox->setUiScale (scale);
        if (cc.speedBox      != nullptr) cc.speedBox->setUiScale (scale);
        if (cc.depthBox      != nullptr) cc.depthBox->setUiScale (scale);
    }

    auto area = getLocalBounds().reduced (scaled (5));
    area.removeFromTop (scaled (40));

    const int boxWidth    = scaled (68);
    const int padding     = scaled (10);
    const int verticalPad = scaled (8);

    // MUST match paint()'s layout exactly
    const float sideFactor = 0.85f;
    const int totalFactor = (int)((4 + 2 * sideFactor) * 100);
    const int baseW = (int)((float)area.getWidth() * 100.0f / (float)totalFactor);
    const int voiceW = baseW;
    const int sideW = (int)((float)baseW * sideFactor);

    juce::Rectangle<int> originalCol = area.removeFromLeft (sideW);
    juce::Rectangle<int> voiceArea   = area.removeFromLeft (voiceW * 4);
    juce::Rectangle<int> lowCutCol   = area.removeFromLeft (sideW);

    const int voicePanelH = juce::jmin (voiceArea.getHeight(), getVoiceColumnContentHeight());

    // Space below the Voice panels goes to the StereoGraph
    auto belowVoices = voiceArea.withTrimmedTop (voicePanelH + scaled (5));
    graph.setBounds (belowVoices.reduced (scaled (2), 0));
    voiceArea.removeFromBottom (scaled (5));

    const int colW = voiceW;
    const int headerH = scaled (28);
    const int topPad = scaled (8);

    juce::Rectangle<int> inputMeterBounds;

    // ---------- ORIGINAL ----------
    {
        auto col = originalCol.reduced (padding, verticalPad);

        col.removeFromTop (topPad);
        originalHeaderLabel.setBounds (col.removeFromTop (headerH));
        auto toggleRow = col.removeFromTop (scaled (28));
        originalOnButton.setBounds (toggleRow.withSizeKeepingCentre (scaled (90), scaled (25)));
        auto knobArea = col.removeFromTop (scaled (100));
        originalVolumeLabel.setBounds (knobArea.removeFromTop (scaled (20)));
        originalVolumeKnob.setBounds (knobArea.expanded (scaled (6), 0));
        col.removeFromTop (scaled (10));

        inputMeterBounds = col.withSizeKeepingCentre (juce::jmin (col.getWidth(), scaled (90)), col.getHeight());
        inputMeter->setBounds (inputMeterBounds);
    }


// ---------- VOICES ----------
    for (int i = 0; i < SvarogWiderAudioProcessor::kNumCopies; ++i)
    {
        auto& cc = copyControls[i];
        auto col = voiceArea.removeFromLeft (colW).withHeight (voicePanelH).reduced (padding, verticalPad);

        const int boxH   = scaled (22);
        const int trackH = scaled (24);
        const int lfoH   = scaled (34);
        const int labelW = scaled (45);

        auto row = [&] (int h) { return col.removeFromTop (h); };

        col.removeFromTop (topPad);

        auto headerRow = row (headerH);
        cc.headerLabel.setBounds (headerRow);

        const auto& font = cc.headerLabel.getFont();
        const int textWidth = juce::GlyphArrangement::getStringWidthInt (font, cc.headerLabel.getText());
        const int dotSize = scaled (10);
        const int gap = scaled (25);
        const int groupWidth = dotSize + gap + textWidth;
        const int groupX = headerRow.getCentreX() - groupWidth / 2;

        cc.voiceDot->setBounds (groupX,
                                 headerRow.getY() + (headerRow.getHeight() - dotSize) / 2 + scaled (-5),
                                 dotSize,
                                 dotSize);

        cc.onButton.setBounds (row (scaled (28)).withSizeKeepingCentre (scaled (90), scaled (25)));

        auto knobArea = row (scaled (100));
        cc.volumeLabel.setBounds (knobArea.removeFromTop (scaled (20)));
        cc.volumeKnob.setBounds (knobArea.expanded (scaled (6), 0));

        col.removeFromTop (scaled (8));

        {
            auto r = row (boxH);
            cc.panLabel.setBounds (r.removeFromLeft (labelW));
            cc.panValueBox->setBounds (r.removeFromRight (boxWidth));
            cc.panSlider.setBounds (row (trackH).expanded (scaled (6), 0));
        }

        col.removeFromTop (scaled (6));

        {
            auto r = row (boxH);
            cc.delayLabel.setBounds (r.removeFromLeft (labelW));
            cc.delayValueBox->setBounds (r.removeFromRight (boxWidth));
            cc.delaySlider.setBounds (row (trackH).expanded (scaled (6), 0));
        }

        col.removeFromTop (scaled (2));

        cc.lfoGraph->setBounds (row (lfoH));

        col.removeFromTop (scaled (6));

        {
            auto r = row (boxH);
            cc.detuneLabel.setBounds (r.removeFromLeft (labelW));
            cc.detuneBox->setBounds (r.removeFromRight (boxWidth));
        }

        col.removeFromTop (scaled (4));

        {
            auto r = row (boxH);
            cc.speedLabel.setBounds (r.removeFromLeft (labelW));
            cc.speedBox->setBounds (r.removeFromRight (boxWidth));
        }

        col.removeFromTop (scaled (4));

        {
            auto r = row (boxH);
            cc.depthLabel.setBounds (r.removeFromLeft (labelW));
            cc.depthBox->setBounds (r.removeFromRight (boxWidth));
        }
    }

    // ---------- LOW CUT ----------
    {
        auto col = lowCutCol.reduced (padding, verticalPad);

        col.removeFromTop (topPad);
        lowCutHeaderLabel.setBounds (col.removeFromTop (headerH));
        lowCutOnButton.setBounds (col.removeFromTop (scaled (28)).withSizeKeepingCentre (scaled (90), scaled (25)));

        col.removeFromTop (scaled (8));

        {
            auto r = col.removeFromTop (scaled (22));
            lowCutValueBox->setBounds (r.withSizeKeepingCentre (boxWidth, scaled (22)));
        }

        col.removeFromTop (scaled (8));
        lowCutSlider.setBounds (col.removeFromTop (scaled (24)).expanded (scaled (6), 0));

        col.removeFromTop (scaled (8));
        auto graphBottomY = inputMeterBounds.getY() - scaled (10);
        const int lowCutGraphH = scaled (30);
        auto graphTopY = graphBottomY - lowCutGraphH;

        if (graphTopY < col.getY()) graphTopY = col.getY();

        lowCutGraph->setBounds (col.getX(), graphTopY, col.getWidth(), lowCutGraphH);

        auto outputBounds = juce::Rectangle<int> (
            col.getX(), inputMeterBounds.getY(),
            col.getWidth(), inputMeterBounds.getHeight());

        outputMeter->setBounds (outputBounds.withSizeKeepingCentre (juce::jmin (col.getWidth(), scaled (90)), outputBounds.getHeight()));
    }

    // ---- Master output control (top-right of the header bar) ----
    {
        auto headerArea = getLocalBounds().reduced (scaled (5)).removeFromTop (scaled (40));
        auto rightStrip = headerArea.removeFromRight (scaled (200)).reduced (scaled (6), scaled (8));

        outputValueBox->setBounds (rightStrip.removeFromRight (scaled (80)));
        outputLabel.setBounds (rightStrip.withTrimmedRight (scaled (8)));
    }
}