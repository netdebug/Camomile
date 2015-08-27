/*
  ==============================================================================

    This file was auto-generated by the Introjucer!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "InstanceProcessor.h"
#include "PatchEditor.h"

//==============================================================================
InstanceProcessor::InstanceProcessor() : Instance(string("camomile")),
m_patch(Patch(*this, "Test2.pd", "/Users/Pierre/Desktop/"))
{
    m_parameters.resize(512);
}

InstanceProcessor::~InstanceProcessor()
{
    lock_guard<mutex> guard(m_mutex_list);
    m_listeners.clear();
    lock_guard<mutex> guard2(m_mutex);
    m_parameters.clear();
}

int InstanceProcessor::getNumParameters()
{
    lock_guard<mutex> guard(m_mutex);
    return int(m_parameters.size());
}

const String InstanceProcessor::getParameterName(int index)
{
    lock_guard<mutex> guard(m_mutex);
    return String(m_parameters[index].getFullName());
}

float InstanceProcessor::getParameter(int index)
{
    lock_guard<mutex> guard(m_mutex);
    return m_parameters[index].getNormalizedValue();
}

void InstanceProcessor::setParameter(int index, float newValue)
{
    lock_guard<mutex> guard(m_mutex);
    m_parameters[index].setNormalizedValue(newValue);
}

float InstanceProcessor::getParameterDefaultValue(int index)
{
    lock_guard<mutex> guard(m_mutex);
    return m_parameters[index].getDefaultNormalizedValue();
}

const String InstanceProcessor::getParameterText(int index)
{
    lock_guard<mutex> guard(m_mutex);
    return String(m_parameters[index].getTextForValue(m_parameters[index].getNormalizedValue()));
}

String InstanceProcessor::getParameterText(int index, int size)
{
    lock_guard<mutex> guard(m_mutex);
    return String(m_parameters[index].getTextForValue(m_parameters[index].getNormalizedValue()).c_str(), size);
}

int InstanceProcessor::getParameterNumSteps(int index)
{
    lock_guard<mutex> guard(m_mutex);
    return m_parameters[index].getNumberOfStep();
}

bool InstanceProcessor::isParameterAutomatable(int index) const
{
    lock_guard<mutex> guard(m_mutex);
    return m_parameters[index].isAutomatable();
}

bool InstanceProcessor::isParameterOrientationInverted(int index) const
{
    return false;
}

bool InstanceProcessor::isMetaParameter(int index) const
{
    lock_guard<mutex> guard(m_mutex);
    return m_parameters[index].isMetaParameter();
}


void InstanceProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    lock_guard<mutex> guard(m_mutex);
    prepareDsp(getNumInputChannels(), getNumOutputChannels(), sampleRate, samplesPerBlock);
}

void InstanceProcessor::releaseResources()
{
    lock_guard<mutex> guard(m_mutex);
    releaseDsp();
}

void InstanceProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    lock_guard<mutex> guard(m_mutex);
    for(int i = getNumInputChannels(); i < getNumOutputChannels(); ++i)
    {
        buffer.clear(i, 0, buffer.getNumSamples());
    }
    performDsp(buffer.getNumSamples(),
               getNumInputChannels(), buffer.getArrayOfReadPointers(),
               getNumOutputChannels(), buffer.getArrayOfWritePointers());
}

AudioProcessorEditor* InstanceProcessor::createEditor()
{
    lock_guard<mutex> guard(m_mutex);
    return new PatchEditor(*this);
}

void InstanceProcessor::getStateInformation(MemoryBlock& destData)
{
    lock_guard<mutex> guard(m_mutex);
    XmlElement xml("CamomileSettings");
    xml.setAttribute("name", m_patch.getName());
    xml.setAttribute("path", m_patch.getPath());
    copyXmlToBinary(xml, destData);
}

void InstanceProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    lock_guard<mutex> guard(m_mutex);
    ScopedPointer<XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if(xml != nullptr)
    {
        if(xml->hasTagName("CamomileSettings"))
        {
            String name = xml->getStringAttribute("name");
            String path = xml->getStringAttribute("path");
            
            File file(path + "/" + name);
            loadPatch(file);
        }
    }
}

void InstanceProcessor::loadPatch(const juce::File& file)
{
    suspendProcessing(true);
    if(isSuspended())
    {
        if(true)
        {
            lock_guard<mutex> guard(m_mutex);
            if(file.exists() && file.getFileExtension() == String(".pd"))
            {
                m_patch = Patch(*this,
                                file.getFileName().toStdString(),
                                file.getParentDirectory().getFullPathName().toStdString());
            }
            
            size_t index = 0;
            vector<Gui> objects(m_patch.getGuis());
            for(auto it : objects)
            {
                vector<Parameter> params = it.getParameters();
                for(auto it2 : params)
                {
                    if(index < m_parameters.size())
                    {
                        m_parameters[index++] = it2;
                    }
                }
            }
            for(; index < m_parameters.size(); index++)
            {
                m_parameters[index++] = Parameter();
            }
        }
        
        vector<Listener*> listeners = getListeners();
        for(auto it : listeners)
        {
            it->patchChanged();
        }
        updateHostDisplay();
    }
    
    suspendProcessing(false);
}

void InstanceProcessor::addListener(Listener* listener)
{
    if(listener)
    {
        lock_guard<mutex> guard(m_mutex_list);
        m_listeners.insert(listener);
    }
}

void InstanceProcessor::removeListener(Listener* listener)
{
    if(listener)
    {
        lock_guard<mutex> guard(m_mutex_list);
        m_listeners.erase(listener);
    }
}

vector<InstanceProcessor::Listener*> InstanceProcessor::getListeners() const noexcept
{
    lock_guard<mutex> guard(m_mutex_list);
    return vector<Listener*>(m_listeners.begin(), m_listeners.end());
}

AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new InstanceProcessor();
}


