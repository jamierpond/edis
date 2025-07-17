#include "PluginProcessor.h"
#include "PluginEditor.h"

EdisAudioProcessorEditor::EdisAudioProcessorEditor(
    EdisAudioProcessor& p)
    : AudioProcessorEditor(&p)
{
    addAndMakeVisible(editor);
    setSize(400, 300);
}

void EdisAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void EdisAudioProcessorEditor::resized()
{
    editor.setBounds(getLocalBounds());
}
