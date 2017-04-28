#ifndef OPENPOSE__WRAPPER__WRAPPER_HPP
#define OPENPOSE__WRAPPER__WRAPPER_HPP

#include "../thread/headers.hpp"
#include "wrapperStructFace.hpp"
#include "wrapperStructHand.hpp"
#include "wrapperStructInput.hpp"
#include "wrapperStructOutput.hpp"
#include "wrapperStructPose.hpp"

namespace op
{
    /**
     * Wrapper: OpenPose all-in-one wrapper template class.
     * Wrapper allows the user to set up the input (video, webcam, custom input, etc.), pose, face and/or hands estimation and rendering,
     * and output (integrated small GUI, custom output, etc.).
     *
     * This function can be used in 2 ways:
     *     - Synchronous mode: call the full constructor with your desired input and output workers.
     *     - Asynchronous mode: call the empty constructor Wrapper() + use the emplace and pop functions to push the original frames and
     *       retrieve the processed ones.
     *     - Mix of them:
     *         - Synchronous input + asynchronous output: call the constructor Wrapper(ThreadManagerMode::Synchronous, workersInput, {}, true)
     *         - Asynchronous input + synchronous output: call the constructor
     *           Wrapper(ThreadManagerMode::Synchronous, nullptr, workersOutput, irrelevantBoolean, true)
     */
    template<typename TDatums, typename TWorker = std::shared_ptr<Worker<std::shared_ptr<TDatums>>>, typename TQueue = Queue<std::shared_ptr<TDatums>>>
    class Wrapper
    {
    public:
        /**
         * Constructor.
         * @param threadManagerMode Thread syncronization mode. If set to ThreadManagerMode::Synchronous, everything will run inside the Wrapper. If
         * ThreadManagerMode::Synchronous(In/Out), then input (frames producer) and/or output (GUI, writing results, etc.) will be controlled
         * outside the Wrapper class by the user. See ThreadManagerMode for a detailed explanation of when to use each one.
         */
        explicit Wrapper(const ThreadManagerMode threadManagerMode = ThreadManagerMode::Synchronous);

        /**
         * Destructor.
         * It automatically frees resources.
         */
        ~Wrapper();

        /**
         * Disable multi-threading.
         * Useful for debugging and logging, all the Workers will run in the same thread.
         * Note that workerOnNewThread (argument for setWorkerInput, setWorkerPostProcessing and setWorkerOutput) will not make any effect.
         */
        void disableMultiThreading();

        /**
         * Add an user-defined extra Worker as frames generator.
         * @param worker TWorker to be added.
         * @param workerOnNewThread Whether to add this TWorker on a new thread (if it is computationally demanding) or simply reuse
         * existing threads (for light functions). Set to true if the performance time is unknown.
         */
        void setWorkerInput(const TWorker& worker, const bool workerOnNewThread = true);

        /**
         * Add an user-defined extra Worker as frames post-processor.
         * @param worker TWorker to be added.
         * @param workerOnNewThread Whether to add this TWorker on a new thread (if it is computationally demanding) or simply reuse
         * existing threads (for light functions). Set to true if the performance time is unknown.
         */
        void setWorkerPostProcessing(const TWorker& worker, const bool workerOnNewThread = true);

        /**
         * Add an user-defined extra Worker as frames consumer (custom display and/or saving).
         * @param worker TWorker to be added.
         * @param workerOnNewThread Whether to add this TWorker on a new thread (if it is computationally demanding) or simply reuse
         * existing threads (for light functions). Set to true if the performance time is unknown.
         */
        void setWorkerOutput(const TWorker& worker, const bool workerOnNewThread = true);

        // If output is not required, just use this function until the renderOutput argument. Keep the default values for the other parameters in order not to display/save any output.
        void configure(const WrapperStructPose& wrapperStructPose,
                       // Producer (set producerSharedPtr = nullptr or use the default WrapperStructInput{} to disable any input)
                       const WrapperStructInput& wrapperStructInput,
                       // Consumer (keep default values to disable any output)
                       const WrapperStructOutput& wrapperStructOutput = WrapperStructOutput{});

        // THIS FUNCTION IS NOT IMPLEMENTED YET -> COMING SOON
        // Similar to the previos configure, but it includes hand extraction and rendering
        void configure(const WrapperStructPose& wrapperStructPose,
                       // Hand (use the default WrapperStructHand{} to disable any hand detector)
                       const experimental::WrapperStructHand& wrapperHandStruct,
                       // Producer (set producerSharedPtr = nullptr or use the default WrapperStructInput{} to disable any input)
                       const WrapperStructInput& wrapperStructInput,
                       // Consumer (keep default values to disable any output)
                       const WrapperStructOutput& wrapperStructOutput = WrapperStructOutput{});

        // THIS FUNCTION IS NOT IMPLEMENTED YET -> COMING SOON
        // Similar to the previos configure, but it includes hand extraction and rendering
        void configure(const WrapperStructPose& wrapperStructPose,
                       // Face (use the default WrapperStructFace{} to disable any face detector)
                       const experimental::WrapperStructFace& wrapperStructFace,
                       // Producer (set producerSharedPtr = nullptr or use the default WrapperStructInput{} to disable any input)
                       const WrapperStructInput& wrapperStructInput,
                       // Consumer (keep default values to disable any output)
                       const WrapperStructOutput& wrapperStructOutput = WrapperStructOutput{});

        // THIS FUNCTION IS NOT IMPLEMENTED YET -> COMING SOON
        // Similar to the previos configure, but it includes hand extraction and rendering
        void configure(const WrapperStructPose& wrapperStructPose = WrapperStructPose{},
                       // Face (use the default WrapperStructFace{} to disable any face detector)
                       const experimental::WrapperStructFace& wrapperStructFace = experimental::WrapperStructFace{},
                       // Hand (use the default WrapperStructHand{} to disable any hand detector)
                       const experimental::WrapperStructHand& wrapperHandStruct = experimental::WrapperStructHand{},
                       // Producer (set producerSharedPtr = nullptr or use the default WrapperStructInput{} to disable any input)
                       const WrapperStructInput& wrapperStructInput = WrapperStructInput{},
                       // Consumer (keep default values to disable any output)
                       const WrapperStructOutput& wrapperStructOutput = WrapperStructOutput{});

        /**
         * Function to start multi-threading.
         * Similar to start(), but exec() blocks the thread that calls the function (it saves 1 thread). Use exec() instead of
         * start() if the calling thread will otherwise be waiting for the Wrapper to end.
         */
        void exec();

        /**
         * Function to start multi-threading.
         * Similar to exec(), but start() does not block the thread that calls the function. It just opens new threads, so it
         * lets the user perform other tasks meanwhile on the calling thread.
         */
        void start();

        /**
         * Function to stop multi-threading.
         * It can be called internally or externally.
         */
        void stop();

        /**
         * Whether the Wrapper is running.
         * It will return true after exec() or start() and before stop(), and false otherwise.
         * @return Boolean specifying whether the Wrapper is running.
         */
        bool isRunning() const;

        /**
         * Emplace (move) an element on the first (input) queue.
         * Only valid if ThreadManagerMode::Asynchronous or ThreadManagerMode::AsynchronousIn.
         * If the input queue is full or the Wrapper was stopped, it will return false and not emplace it.
         * @param tDatums std::shared_ptr<TDatums> element to be emplaced.
         * @return Boolean specifying whether the tDatums could be emplaced.
         */
        bool tryEmplace(std::shared_ptr<TDatums>& tDatums);

        /**
         * Emplace (move) an element on the first (input) queue.
         * Similar to tryEmplace.
         * However, if the input queue is full, it will wait until it can emplace it.
         * If the Wrapper class is stopped before adding the element, it will return false and not emplace it.
         * @param tDatums std::shared_ptr<TDatums> element to be emplaced.
         * @return Boolean specifying whether the tDatums could be emplaced.
         */
        bool waitAndEmplace(std::shared_ptr<TDatums>& tDatums);

        /**
         * Push (copy) an element on the first (input) queue.
         * Same as tryEmplace, but it copies the data instead of moving it.
         * @param tDatums std::shared_ptr<TDatums> element to be pushed.
         * @return Boolean specifying whether the tDatums could be pushed.
         */
        bool tryPush(const std::shared_ptr<TDatums>& tDatums);

        /**
         * Push (copy) an element on the first (input) queue.
         * Same as waitAndEmplace, but it copies the data instead of moving it.
         * @param tDatums std::shared_ptr<TDatums> element to be pushed.
         * @return Boolean specifying whether the tDatums could be pushed.
         */
        bool waitAndPush(const std::shared_ptr<TDatums>& tDatums);

        /**
         * Pop (retrieve) an element from the last (output) queue.
         * Only valid if ThreadManagerMode::Asynchronous or ThreadManagerMode::AsynchronousOut.
         * If the output queue is empty or the Wrapper was stopped, it will return false and not retrieve it.
         * @param tDatums std::shared_ptr<TDatums> element where the retrieved element will be placed.
         * @return Boolean specifying whether the tDatums could be retrieved.
         */
        bool tryPop(std::shared_ptr<TDatums>& tDatums);

        /**
         * Pop (retrieve) an element from the last (output) queue.
         * Similar to tryPop.
         * However, if the output queue is empty, it will wait until it can pop an element.
         * If the Wrapper class is stopped before popping the element, it will return false and not retrieve it.
         * @param tDatums std::shared_ptr<TDatums> element where the retrieved element will be placed.
         * @return Boolean specifying whether the tDatums could be retrieved.
         */
        bool waitAndPop(std::shared_ptr<TDatums>& tDatums);

    private:
        const ThreadManagerMode mThreadManagerMode;
        const std::shared_ptr<std::pair<std::atomic<bool>, std::atomic<int>>> spVideoSeek;
        ThreadManager<std::shared_ptr<TDatums>> mThreadManager;
        int mGpuNumber;
        bool mUserInputWsOnNewThread;
        bool mUserPostProcessingWsOnNewThread;
        bool mUserOutputWsOnNewThread;
        unsigned int mThreadId;
        bool mMultiThreadEnabled;
        // Workers
        std::vector<TWorker> mUserInputWs;
        TWorker wDatumProducer;
        TWorker spWIdGenerator;
        TWorker spWCvMatToOpInput;
        TWorker spWCvMatToOpOutput;
        std::vector<std::vector<TWorker>> spWPoses;
        std::vector<TWorker> mPostProcessingWs;
        std::vector<TWorker> mUserPostProcessingWs;
        std::vector<TWorker> mOutputWs;
        TWorker spWGui;
        std::vector<TWorker> mUserOutputWs;

        /**
         * Frees TWorker variables (private internal function).
         * For most cases, this class is non-necessary, since std::shared_ptr are automatically cleaned on destruction of each class.
         * However, it might be useful if the same Wrapper is gonna be started twice (not recommended on most cases).
         */
        void reset();

        /**
         * Set ThreadManager from TWorkers (private internal function).
         * After any configure() has been called, the TWorkers are initialized. This function resets the ThreadManager and adds them. 
         * Common code for start() and exec().
         */
        void configureThreadManager();

        /**
         * Thread ID increase (private internal function).
         * If multi-threading mode, it increases the thread ID.
         * If single-threading mode (for debugging), it does not modify it.
         * Note that mThreadId must be re-initialized to 0 before starting a new Wrapper configuration.
         * @return unsigned int with the next thread id value.
         */
        unsigned int threadIdPP();

        /**
         * TWorker concatenator (private internal function).
         * Auxiliary function that concatenate std::vectors of TWorker. Since TWorker is some kind of smart pointer (usually
         * std::shared_ptr), its copy still shares the same internal data. It will not work for TWorker classes that do not share
         * the data when moved.
         * @param workersA First std::shared_ptr<TDatums> element to be concatenated.
         * @param workersB Second std::shared_ptr<TDatums> element to be concatenated.
         * @return Concatenated std::vector<TWorker> of both workersA and workersB.
         */
        std::vector<TWorker> mergeWorkers(const std::vector<TWorker>& workersA, const std::vector<TWorker>& workersB);

        DELETE_COPY(Wrapper);
    };
}





// Implementation
#include "../core/headers.hpp"
#include "../experimental/headers.hpp"
#include "../filestream/headers.hpp"
#include "../gui/headers.hpp"
#include "../pose/headers.hpp"
#include "../producer/headers.hpp"
#include "../utilities/errorAndLog.hpp"
#include "../utilities/fileSystem.hpp"
namespace op
{
    template<typename TDatums, typename TWorker, typename TQueue>
    Wrapper<TDatums, TWorker, TQueue>::Wrapper(const ThreadManagerMode threadManagerMode) :
        mThreadManagerMode{threadManagerMode},
        spVideoSeek{std::make_shared<std::pair<std::atomic<bool>, std::atomic<int>>>()},
        mThreadManager{threadManagerMode},
        mMultiThreadEnabled{true}
    {
        try
        {
            // It cannot be directly included in the constructor, otherwise compiler error for copying std::atomic
            spVideoSeek->first = false;
            spVideoSeek->second = 0;
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    template<typename TDatums, typename TWorker, typename TQueue>
    Wrapper<TDatums, TWorker, TQueue>::~Wrapper()
    {
        try
        {
            stop();
            reset();
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    template<typename TDatums, typename TWorker, typename TQueue>
    void Wrapper<TDatums, TWorker, TQueue>::disableMultiThreading()
    {
        try
        {
            mMultiThreadEnabled = false;
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    template<typename TDatums, typename TWorker, typename TQueue>
    void Wrapper<TDatums, TWorker, TQueue>::setWorkerInput(const TWorker& worker, const bool workerOnNewThread)
    {
        try
        {
            mUserInputWs.clear();
            if (worker == nullptr)
                error("Your worker is a nullptr.", __LINE__, __FILE__, __FUNCTION__);
            mUserInputWs.emplace_back(worker);
            mUserInputWsOnNewThread = {workerOnNewThread};
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    template<typename TDatums, typename TWorker, typename TQueue>
    void Wrapper<TDatums, TWorker, TQueue>::setWorkerPostProcessing(const TWorker& worker, const bool workerOnNewThread)
    {
        try
        {
            mUserPostProcessingWs.clear();
            if (worker == nullptr)
                error("Your worker is a nullptr.", __LINE__, __FILE__, __FUNCTION__);
            mUserPostProcessingWs.emplace_back(worker);
            mUserPostProcessingWsOnNewThread = {workerOnNewThread};
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    template<typename TDatums, typename TWorker, typename TQueue>
    void Wrapper<TDatums, TWorker, TQueue>::setWorkerOutput(const TWorker& worker, const bool workerOnNewThread)
    {
        try
        {
            mUserOutputWs.clear();
            if (worker == nullptr)
                error("Your worker is a nullptr.", __LINE__, __FILE__, __FUNCTION__);
            mUserOutputWs.emplace_back(worker);
            mUserOutputWsOnNewThread = {workerOnNewThread};
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    template<typename TDatums, typename TWorker, typename TQueue>
    void Wrapper<TDatums, TWorker, TQueue>::configure(const WrapperStructPose& wrapperStructPose, const WrapperStructInput& wrapperStructInput,
                                                      const WrapperStructOutput& wrapperStructOutput)
    {
        try
        {
            configure(wrapperStructPose, experimental::WrapperStructFace{}, experimental::WrapperStructHand{}, wrapperStructInput, wrapperStructOutput);
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    template<typename TDatums, typename TWorker, typename TQueue>
    void Wrapper<TDatums, TWorker, TQueue>::configure(const WrapperStructPose& wrapperStructPose, const experimental::WrapperStructFace& wrapperStructFace,
                                                      const WrapperStructInput& wrapperStructInput, const WrapperStructOutput& wrapperStructOutput)
    {
        try
        {
            configure(wrapperStructPose, wrapperStructFace, experimental::WrapperStructHand{}, wrapperStructInput, wrapperStructOutput);
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    template<typename TDatums, typename TWorker, typename TQueue>
    void Wrapper<TDatums, TWorker, TQueue>::configure(const WrapperStructPose& wrapperStructPose, const experimental::WrapperStructHand& wrapperHandStruct,
                                                      const WrapperStructInput& wrapperStructInput, const WrapperStructOutput& wrapperStructOutput)
    {
        try
        {
            configure(wrapperStructPose, experimental::WrapperStructFace{}, wrapperHandStruct, wrapperStructInput, wrapperStructOutput);
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    template<typename TDatums, typename TWorker, typename TQueue>
    void Wrapper<TDatums, TWorker, TQueue>::configure(const WrapperStructPose& wrapperStructPose, const experimental::WrapperStructFace& wrapperStructFace,
                                                      const experimental::WrapperStructHand& wrapperHandStruct, const WrapperStructInput& wrapperStructInput,
                                                      const WrapperStructOutput& wrapperStructOutput)
    {
        try
        {
            log("", Priority::Low, __LINE__, __FUNCTION__, __FILE__);

            // Shortcut
            typedef std::shared_ptr<TDatums> TDatumsPtr;

            // Check no contradictory flags enabled
            if (wrapperStructPose.alphaPose < 0. || wrapperStructPose.alphaPose > 1. || wrapperStructPose.alphaHeatMap < 0.
                || wrapperStructPose.alphaHeatMap > 1.)
                error("Alpha value for blending must be in the range [0,1].", __LINE__, __FUNCTION__, __FILE__);
            if (wrapperStructPose.scaleGap <= 0.f && wrapperStructPose.scalesNumber > 1)
                error("The scale gap must be greater than 0 (it has no effect if the number of scales is 1).", __LINE__, __FUNCTION__, __FILE__);
            if (!wrapperStructPose.renderOutput && (!wrapperStructOutput.writeImages.empty() || !wrapperStructOutput.writeVideo.empty()))
            {
                const auto message = "In order to save the rendered frames (`write_images` or `write_video`), you must set `render_output` to true.";
                error(message, __LINE__, __FUNCTION__, __FILE__);
            }
            if (!wrapperStructOutput.writeHeatMaps.empty() && wrapperStructPose.heatMapTypes.empty())
            {
                const auto message = "In order to save the heatmaps (`write_heatmaps`), you need to pick which heat maps you want to save: `heatmaps_add_X`"
                                     " flags or fill the wrapperStructPose.heatMapTypes.";
                error(message, __LINE__, __FUNCTION__, __FILE__);
            }
            if (!wrapperStructOutput.writeHeatMaps.empty() && wrapperStructPose.heatMapScaleMode != ScaleMode::UnsignedChar)
            {
                const auto message = "In order to save the heatmaps, you must set wrapperStructPose.heatMapScaleMode to ScaleMode::UnsignedChar,"
                                     " i.e. range [0, 255].";
                error(message, __LINE__, __FUNCTION__, __FILE__);
            }
            if (mUserOutputWs.empty() && mThreadManagerMode != ThreadManagerMode::Asynchronous && mThreadManagerMode != ThreadManagerMode::AsynchronousOut)
            {
                const std::string additionalMessage = " You could also set mThreadManagerMode = mThreadManagerMode::Asynchronous(Out) and/or add your own"
                                                      " output worker class before calling this function.";
                const auto savingSomething = (!wrapperStructOutput.writeImages.empty() || !wrapperStructOutput.writeVideo.empty()
                                              || !wrapperStructOutput.writePose.empty() || !wrapperStructOutput.writePoseJson.empty()
                                              || !wrapperStructOutput.writeCocoJson.empty() || !wrapperStructOutput.writeHeatMaps.empty());
                if (!wrapperStructOutput.displayGui && !savingSomething)
                {
                    const auto message = "No output is selected (`no_display`) and no results are generated (no `write_X` flags enabled). Thus,"
                                         " no output would be generated." + additionalMessage;
                    error(message, __LINE__, __FUNCTION__, __FILE__);
                }

                if ((wrapperStructOutput.displayGui && wrapperStructOutput.guiVerbose) && !wrapperStructPose.renderOutput)
                {
                    const auto message = "No render is enabled (`no_render_output`), so you should also remove the display (set `no_display`"
                                         " or `no_gui_verbose`)." + additionalMessage;
                    error(message, __LINE__, __FUNCTION__, __FILE__);
                }
                if (wrapperStructInput.framesRepeat && savingSomething)
                {
                    const auto message = "Frames repetition (`frames_repeat`) is enabled as well as some writing function (`write_X`). This program would"
                                         " never stop recording the same frames over and over. Please, disable repetition or remove writing.";
                    error(message, __LINE__, __FUNCTION__, __FILE__);
                }
                if (wrapperStructInput.realTimeProcessing && savingSomething)
                {
                    const auto message = "Real time processing is enabled as well as some writing function. Thus, some frames might be skipped. Consider"
                                         " disabling real time processing if you intend to save any results.";
                    log(message, Priority::Max, __LINE__, __FUNCTION__, __FILE__);
                }
            }
            if (!wrapperStructOutput.writeVideo.empty() && wrapperStructInput.producerSharedPtr == nullptr)
                error("Writting video is only available if the OpenPose producer is used (i.e. wrapperStructInput.producerSharedPtr cannot be a nullptr).");

            // Proper format
            const auto writeImagesCleaned = formatAsDirectory(wrapperStructOutput.writeImages);
            const auto writePoseCleaned = formatAsDirectory(wrapperStructOutput.writePose);
            const auto writePoseJsonCleaned = formatAsDirectory(wrapperStructOutput.writePoseJson);
            const auto writeHeatMapsCleaned = formatAsDirectory(wrapperStructOutput.writeHeatMaps);

            // Common parameters
            auto finalOutputSize = wrapperStructPose.outputSize;
            cv::Size producerSize{-1,-1};
            if (wrapperStructInput.producerSharedPtr != nullptr)
            {
                // 1. Set producer properties
                const auto displayProducerFpsMode = (wrapperStructInput.realTimeProcessing ? ProducerFpsMode::OriginalFps : ProducerFpsMode::RetrievalFps);
                wrapperStructInput.producerSharedPtr->setProducerFpsMode(displayProducerFpsMode);
                wrapperStructInput.producerSharedPtr->set(ProducerProperty::Flip, wrapperStructInput.frameFlip);
                wrapperStructInput.producerSharedPtr->set(ProducerProperty::Rotation, wrapperStructInput.frameRotate);
                wrapperStructInput.producerSharedPtr->set(ProducerProperty::AutoRepeat, wrapperStructInput.framesRepeat);
                // 2. Set finalOutputSize
                producerSize = cv::Size{(int)wrapperStructInput.producerSharedPtr->get(CV_CAP_PROP_FRAME_WIDTH),
                                        (int)wrapperStructInput.producerSharedPtr->get(CV_CAP_PROP_FRAME_HEIGHT)};
                if (wrapperStructPose.outputSize.width == -1 || wrapperStructPose.outputSize.height == -1)
                {
                    if (producerSize.area() > 0)
                        finalOutputSize = producerSize;
                    else
                    {
                        const auto message = "Output resolution = input resolution not valid for image reading (size might change between images).";
                        error(message, __LINE__, __FUNCTION__, __FILE__);
                    }
                }
            }
            else if (finalOutputSize.width == -1 || finalOutputSize.height == -1)
            {
                const auto message = "Output resolution cannot be (-1 x -1) unless wrapperStructInput.producerSharedPtr is also set.";
                error(message, __LINE__, __FUNCTION__, __FILE__);
            }

            // Update global parameter
            mGpuNumber = wrapperStructPose.gpuNumber;

            // Producer
            if (wrapperStructInput.producerSharedPtr != nullptr)
            {
                const auto datumProducer = std::make_shared<DatumProducer<TDatums>>(
                    wrapperStructInput.producerSharedPtr, wrapperStructInput.frameFirst, wrapperStructInput.frameLast, spVideoSeek
                );
                wDatumProducer = std::make_shared<WDatumProducer<TDatumsPtr, TDatums>>(datumProducer);
            }
            else
                wDatumProducer = nullptr;

            // Pose estimators
            const cv::Size& netOutputSize = wrapperStructPose.netInputSize;
            std::vector<std::shared_ptr<PoseExtractor>> poseExtractors;
            for (auto gpuId = 0; gpuId < wrapperStructPose.gpuNumber; gpuId++)
                poseExtractors.emplace_back(std::make_shared<PoseExtractorCaffe>(
                    wrapperStructPose.netInputSize, netOutputSize, finalOutputSize, wrapperStructPose.scalesNumber,
                    wrapperStructPose.scaleGap, wrapperStructPose.poseModel, wrapperStructPose.modelFolder,
                    gpuId + wrapperStructPose.gpuNumberStart, wrapperStructPose.heatMapTypes, wrapperStructPose.heatMapScaleMode
                ));
            // Pose renderers
            std::vector<std::shared_ptr<PoseRenderer>> poseRenderers;
            if (wrapperStructPose.renderOutput)
            {
                for (auto gpuId = 0; gpuId < poseExtractors.size(); gpuId++)
                {
                    poseRenderers.emplace_back(std::make_shared<PoseRenderer>(
                        netOutputSize, finalOutputSize, wrapperStructPose.poseModel, poseExtractors[gpuId],
                        wrapperStructPose.blendOriginalFrame, wrapperStructPose.alphaPose,
                        wrapperStructPose.alphaHeatMap, wrapperStructPose.defaultPartToRender
                    ));
                }
            }
            log("", Priority::Low, __LINE__, __FUNCTION__, __FILE__);

            // Input cvMat to OpenPose format
            const auto cvMatToOpInput = std::make_shared<CvMatToOpInput>(
                wrapperStructPose.netInputSize, wrapperStructPose.scalesNumber, wrapperStructPose.scaleGap
            );
            spWCvMatToOpInput = std::make_shared<WCvMatToOpInput<TDatumsPtr>>(cvMatToOpInput);
            const auto cvMatToOpOutput = std::make_shared<CvMatToOpOutput>(finalOutputSize, wrapperStructPose.renderOutput);
            spWCvMatToOpOutput = std::make_shared<WCvMatToOpOutput<TDatumsPtr>>(cvMatToOpOutput);

            // Pose extractor(s)
            spWPoses.clear();
            spWPoses.resize(poseExtractors.size());
            for (auto i = 0; i < spWPoses.size(); i++)
                spWPoses.at(i) = {std::make_shared<WPoseExtractor<TDatumsPtr>>(poseExtractors.at(i))};

            // Face extractor(s)
            if (wrapperStructFace.extractAndRenderFace)
            {
                for (auto gpuId = 0; gpuId < spWPoses.size(); gpuId++)
                {
                    const auto faceExtractor = std::make_shared<experimental::FaceExtractor>(
                        wrapperStructPose.modelFolder, gpuId + wrapperStructPose.gpuNumberStart, wrapperStructPose.poseModel
                    );
                    spWPoses.at(gpuId).emplace_back(std::make_shared<experimental::WFaceExtractor<TDatumsPtr>>(faceExtractor));
                }
            }

            // Hand extractor(s)
            if (wrapperHandStruct.extractAndRenderHands)
            {
                for (auto gpuId = 0; gpuId < spWPoses.size(); gpuId++)
                {
                    const auto handExtractor = std::make_shared<experimental::HandExtractor>(
                        wrapperStructPose.modelFolder, gpuId + wrapperStructPose.gpuNumberStart, wrapperStructPose.poseModel
                    );
                    spWPoses.at(gpuId).emplace_back(std::make_shared<experimental::WHandExtractor<TDatumsPtr>>(handExtractor));
                }
            }

            // Pose renderer(s)
            if (!poseRenderers.empty())
                for (auto i = 0; i < spWPoses.size(); i++)
                    spWPoses.at(i).emplace_back(std::make_shared<WPoseRenderer<TDatumsPtr>>(poseRenderers.at(i)));

            // Hands renderer(s)
            if (wrapperHandStruct.extractAndRenderHands)
            {
                for (auto i = 0; i < spWPoses.size(); i++)
                {
                    // Construct hands renderer
                    const auto handRenderer = std::make_shared<experimental::HandRenderer>(finalOutputSize);
                    // Performance boost -> share spGpuMemoryPtr for all renderers
                    if (!poseRenderers.empty())
                    {
                        const bool isLastRenderer = (!wrapperStructFace.extractAndRenderFace);
                        handRenderer->setGpuMemoryAndSetIfLast(poseRenderers.at(i)->getGpuMemoryAndSetAsFirst(), isLastRenderer);
                    }
                    // Add worker
                    spWPoses.at(i).emplace_back(std::make_shared<experimental::WHandRenderer<TDatumsPtr>>(handRenderer));
                }
            }

            // Face renderer(s)
            if (wrapperStructFace.extractAndRenderFace)
            {
                for (auto i = 0; i < spWPoses.size(); i++)
                {
                    // Construct face renderer
                    const auto faceRenderer = std::make_shared<experimental::FaceRenderer>(finalOutputSize);
                    // Performance boost -> share spGpuMemoryPtr for all renderers
                    if (!poseRenderers.empty())
                    {
                        const bool isLastRenderer = true;
                        faceRenderer->setGpuMemoryAndSetIfLast(poseRenderers.at(i)->getGpuMemoryAndSetAsFirst(), isLastRenderer);
                    }
                    // Add worker
                    spWPoses.at(i).emplace_back(std::make_shared<experimental::WFaceRenderer<TDatumsPtr>>(faceRenderer));
                }
            }

            // Itermediate workers (e.g. OpenPose format to cv::Mat, json & frames recorder, ...)
            mPostProcessingWs.clear();
            // Frame buffer and ordering
            if (spWPoses.size() > 1)
                mPostProcessingWs.emplace_back(std::make_shared<WQueueOrderer<TDatumsPtr>>());
            // Frames processor (OpenPose format -> cv::Mat format)
            if (wrapperStructPose.renderOutput)
            {
                const auto opOutputToCvMat = std::make_shared<OpOutputToCvMat>(finalOutputSize);
                mPostProcessingWs.emplace_back(std::make_shared<WOpOutputToCvMat<TDatumsPtr>>(opOutputToCvMat));
            }
            // Re-scale pose if desired
            if (wrapperStructPose.poseScaleMode != ScaleMode::OutputResolution
                && (wrapperStructPose.poseScaleMode != ScaleMode::InputResolution || (finalOutputSize != producerSize))
                && (wrapperStructPose.poseScaleMode != ScaleMode::NetOutputResolution || (finalOutputSize != netOutputSize)))
            {
                auto keyPointScaler = std::make_shared<KeyPointScaler>(wrapperStructPose.poseScaleMode);
                mPostProcessingWs.emplace_back(std::make_shared<WKeyPointScaler<TDatumsPtr>>(keyPointScaler));
            }

            mOutputWs.clear();
            // Write people pose data on disk (json for OpenCV >= 3, xml, yml...)
            if (!writePoseCleaned.empty())
            {
                const auto poseSaver = std::make_shared<PoseSaver>(writePoseCleaned, wrapperStructOutput.writePoseDataFormat);
                mOutputWs.emplace_back(std::make_shared<WPoseSaver<TDatumsPtr>>(poseSaver));
            }
            // Write people pose data on disk (json format)
            if (!writePoseJsonCleaned.empty())
            {
                const auto poseJsonSaver = std::make_shared<PoseJsonSaver>(writePoseJsonCleaned);
                mOutputWs.emplace_back(std::make_shared<WPoseJsonSaver<TDatumsPtr>>(poseJsonSaver));
            }
            // Write people pose data on disk (COCO validation json format)
            if (!wrapperStructOutput.writeCocoJson.empty())
            {
                const auto humanFormat = true; // If true, bigger size (and potentially slower to process), but easier for a human to read it
                const auto poseJsonCocoSaver = std::make_shared<PoseJsonCocoSaver>(wrapperStructOutput.writeCocoJson, humanFormat);
                mOutputWs.emplace_back(std::make_shared<experimental::WPoseJsonCocoSaver<TDatumsPtr>>(poseJsonCocoSaver));
            }
            // Write frames as desired image format on hard disk
            if (!writeImagesCleaned.empty())
            {
                const auto imageSaver = std::make_shared<ImageSaver>(writeImagesCleaned, wrapperStructOutput.writeImagesFormat);
                mOutputWs.emplace_back(std::make_shared<WImageSaver<TDatumsPtr>>(imageSaver));
            }
            // Write frames as *.avi video on hard disk
            if (!wrapperStructOutput.writeVideo.empty() && wrapperStructInput.producerSharedPtr != nullptr)
            {
                const auto originalVideoFps = (wrapperStructInput.producerSharedPtr->getType() != ProducerType::Webcam
                                               && wrapperStructInput.producerSharedPtr->get(CV_CAP_PROP_FPS) > 0.
                                               ? wrapperStructInput.producerSharedPtr->get(CV_CAP_PROP_FPS) : 30.);
                const auto videoSaver = std::make_shared<VideoSaver>(
                    wrapperStructOutput.writeVideo, CV_FOURCC('M','J','P','G'), originalVideoFps, finalOutputSize
                );
                mOutputWs.emplace_back(std::make_shared<WVideoSaver<TDatumsPtr>>(videoSaver));
            }
            // Write heat maps as desired image format on hard disk
            if (!writeHeatMapsCleaned.empty())
            {
                const auto heatMapSaver = std::make_shared<HeatMapSaver>(writeHeatMapsCleaned, wrapperStructOutput.writeHeatMapsFormat);
                mOutputWs.emplace_back(std::make_shared<WHeatMapSaver<TDatumsPtr>>(heatMapSaver));
            }
            // Add frame information for GUI
            // If this WGuiInfoAdder instance is placed before the WImageSaver or WVideoSaver, then the resulting recorded frames will
            // look exactly as the final displayed image by the GUI
            if (wrapperStructOutput.displayGui && wrapperStructOutput.guiVerbose)
            {
                const auto guiInfoAdder = std::make_shared<GuiInfoAdder>(finalOutputSize, wrapperStructPose.gpuNumber);
                mOutputWs.emplace_back(std::make_shared<WGuiInfoAdder<TDatumsPtr>>(guiInfoAdder));
            }
            // Minimal graphical user interface (GUI)
            spWGui = nullptr;
            if (wrapperStructOutput.displayGui)
            {
                const auto gui = std::make_shared<Gui>(
                    wrapperStructOutput.fullScreen, finalOutputSize, mThreadManager.getIsRunningSharedPtr(), spVideoSeek, poseExtractors, poseRenderers
                );
                spWGui = {std::make_shared<WGui<TDatumsPtr>>(gui)};
            }
            log("", Priority::Low, __LINE__, __FUNCTION__, __FILE__);
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    template<typename TDatums, typename TWorker, typename TQueue>
    void Wrapper<TDatums, TWorker, TQueue>::exec()
    {
        try
        {
            configureThreadManager();
            mThreadManager.exec();
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    template<typename TDatums, typename TWorker, typename TQueue>
    void Wrapper<TDatums, TWorker, TQueue>::start()
    {
        try
        {
            configureThreadManager();
            mThreadManager.start();
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    template<typename TDatums, typename TWorker, typename TQueue>
    void Wrapper<TDatums, TWorker, TQueue>::stop()
    {
        try
        {
            mThreadManager.stop();
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    template<typename TDatums, typename TWorker, typename TQueue>
    bool Wrapper<TDatums, TWorker, TQueue>::isRunning() const
    {
        try
        {
            return mThreadManager.isRunning();
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
            return false;
        }
    }

    template<typename TDatums, typename TWorker, typename TQueue>
    bool Wrapper<TDatums, TWorker, TQueue>::tryEmplace(std::shared_ptr<TDatums>& tDatums)
    {
        try
        {
            if (!mUserInputWs.empty())
                error("Emplace cannot be called if an input worker was already selected.", __LINE__, __FUNCTION__, __FILE__);
            return mThreadManager.tryEmplace(tDatums);
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
            return false;
        }
    }

    template<typename TDatums, typename TWorker, typename TQueue>
    bool Wrapper<TDatums, TWorker, TQueue>::waitAndEmplace(std::shared_ptr<TDatums>& tDatums)
    {
        try
        {
            if (!mUserInputWs.empty())
                error("Emplace cannot be called if an input worker was already selected.", __LINE__, __FUNCTION__, __FILE__);
            return mThreadManager.waitAndEmplace(tDatums);
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
            return false;
        }
    }

    template<typename TDatums, typename TWorker, typename TQueue>
    bool Wrapper<TDatums, TWorker, TQueue>::tryPush(const std::shared_ptr<TDatums>& tDatums)
    {
        try
        {
            if (!mUserInputWs.empty())
                error("Push cannot be called if an input worker was already selected.", __LINE__, __FUNCTION__, __FILE__);
            return mThreadManager.tryPush(tDatums);
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
            return false;
        }
    }

    template<typename TDatums, typename TWorker, typename TQueue>
    bool Wrapper<TDatums, TWorker, TQueue>::waitAndPush(const std::shared_ptr<TDatums>& tDatums)
    {
        try
        {
            if (!mUserInputWs.empty())
                error("Push cannot be called if an input worker was already selected.", __LINE__, __FUNCTION__, __FILE__);
            return mThreadManager.waitAndPush(tDatums);
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
            return false;
        }
    }

    template<typename TDatums, typename TWorker, typename TQueue>
    bool Wrapper<TDatums, TWorker, TQueue>::tryPop(std::shared_ptr<TDatums>& tDatums)
    {
        try
        {
            if (!mUserOutputWs.empty())
                error("Pop cannot be called if an output worker was already selected.", __LINE__, __FUNCTION__, __FILE__);
            return mThreadManager.tryPop(tDatums);
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
            return false;
        }
    }

    template<typename TDatums, typename TWorker, typename TQueue>
    bool Wrapper<TDatums, TWorker, TQueue>::waitAndPop(std::shared_ptr<TDatums>& tDatums)
    {
        try
        {
            if (!mUserOutputWs.empty())
                error("Pop cannot be called if an output worker was already selected.", __LINE__, __FUNCTION__, __FILE__);
            return mThreadManager.waitAndPop(tDatums);
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
            return false;
        }
    }

    template<typename TDatums, typename TWorker, typename TQueue>
    void Wrapper<TDatums, TWorker, TQueue>::reset()
    {
        try
        {
            mThreadManager.reset();
            mThreadId = 0ull;
            // Reset 
            mUserInputWs.clear();
            wDatumProducer = nullptr;
            spWCvMatToOpInput = nullptr;
            spWCvMatToOpOutput = nullptr;
            spWPoses.clear();
            mPostProcessingWs.clear();
            mUserPostProcessingWs.clear();
            mOutputWs.clear();
            spWGui = nullptr;
            mUserOutputWs.clear();
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    template<typename TDatums, typename TWorker, typename TQueue>
    void Wrapper<TDatums, TWorker, TQueue>::configureThreadManager()
    {
        try
        {
            // The less number of queues -> the less lag

            // Security checks
            if (spWCvMatToOpInput == nullptr || spWCvMatToOpOutput == nullptr)
                error("Configure the Wrapper class before calling `start()`.", __LINE__, __FUNCTION__, __FILE__);
            if ((wDatumProducer == nullptr) == (mUserInputWs.empty())
                && mThreadManagerMode != ThreadManagerMode::Asynchronous && mThreadManagerMode != ThreadManagerMode::AsynchronousIn)
            {
                const auto message = "You need to have 1 and only 1 producer selected. You can introduce your own producer by using setWorkerInput() or"
                                     " use the OpenPose default producer by configuring it in the configure function) or use the"
                                     " ThreadManagerMode::Asynchronous(In) mode.";
                error(message, __LINE__, __FUNCTION__, __FILE__);
            }
            if (mOutputWs.empty() && mUserOutputWs.empty() && spWGui == nullptr && mThreadManagerMode != ThreadManagerMode::Asynchronous
                && mThreadManagerMode != ThreadManagerMode::AsynchronousOut)
            {
                error("No output selected.", __LINE__, __FUNCTION__, __FILE__);
            }

            // Thread Manager:
            // Clean previous thread manager (avoid configure to crash the program if used more than once)
            mThreadManager.reset();
            mThreadId = 0ull;
            auto queueIn = 0ull;
            auto queueOut = 1ull;
            // If custom user Worker and uses its own thread
            spWIdGenerator = std::make_shared<WIdGenerator<std::shared_ptr<TDatums>>>();
            if (!mUserInputWs.empty() && mUserInputWsOnNewThread)
            {
                mThreadManager.add(mThreadId, mUserInputWs, queueIn++, queueOut++);                     // Thread 0, queues 0 -> 1
                threadIdPP();
                mThreadManager.add(mThreadId, {spWIdGenerator, spWCvMatToOpInput, spWCvMatToOpOutput}, queueIn++, queueOut++); // Thread 1, queues 1 -> 2
            }
            // If custom user Worker in same thread or producer on same thread
            else
            {
                std::vector<TWorker> workersAux;
                // Custom user Worker
                if (!mUserInputWs.empty())
                    workersAux = mergeWorkers(workersAux, mUserInputWs);
                // OpenPose producer
                else if (wDatumProducer != nullptr)       
                    workersAux = mergeWorkers(workersAux, {wDatumProducer});
                // Otherwise
                else if (mThreadManagerMode != ThreadManagerMode::Asynchronous && mThreadManagerMode != ThreadManagerMode::AsynchronousIn)
                    error("No input selected.", __LINE__, __FUNCTION__, __FILE__);

                workersAux = mergeWorkers(workersAux, {spWIdGenerator, spWCvMatToOpInput, spWCvMatToOpOutput});
                mThreadManager.add(mThreadId, workersAux, queueIn++, queueOut++);                       // Thread 0 or 1, queues 0 -> 1
            }
            threadIdPP();
            // Pose estimation & rendering
            if (!spWPoses.empty())                                                                      // Thread 1 or 2...X, queues 1 -> 2, X = 2 + #GPUs
            {
                if (mMultiThreadEnabled)
                {
                    for (auto& wPose : spWPoses)
                    {
                        mThreadManager.add(mThreadId, wPose, queueIn, queueOut);
                        threadIdPP();
                    }
                }
                else
                {
                    log("Debugging activated, only 1 thread running, all spWPoses have been disabled but the first one.");
                    mThreadManager.add(mThreadId, spWPoses.at(0), queueIn, queueOut);
                }
                queueIn++;
                queueOut++;
            }
            // If custom user Worker and uses its own thread
            if (!mUserPostProcessingWs.empty() && mUserPostProcessingWsOnNewThread)
            {
                // Post processing workers
                if (!mPostProcessingWs.empty())
                {
                    mThreadManager.add(mThreadId, mPostProcessingWs, queueIn++, queueOut++);                // Thread 2 or 3, queues 2 -> 3
                    threadIdPP();
                }
                // User processing workers
                mThreadManager.add(mThreadId, mUserPostProcessingWs, queueIn++, queueOut++);                // Thread 3 or 4, queues 3 -> 4
                threadIdPP();
                // Output workers
                if (!mOutputWs.empty())
                {
                    mThreadManager.add(mThreadId, mOutputWs, queueIn++, queueOut++);                        // Thread 4 or 5, queues 4 -> 5
                    threadIdPP();
                }
            }
            // If custom user Worker in same thread or producer on same thread
            else
            {
                // Post processing workers + User post processing workers + Output workers
                auto workersAux = mergeWorkers(mPostProcessingWs, mUserPostProcessingWs);
                workersAux = mergeWorkers(workersAux, mOutputWs);
                if (!workersAux.empty())
                {
                    mThreadManager.add(mThreadId, workersAux, queueIn++, queueOut++);                       // Thread 2 or 3, queues 2 -> 3
                    threadIdPP();
                }
            }
            // User output worker
            if (!mUserOutputWs.empty())                                                                     // Thread Y, queues Q -> Q+1
            {
                if (mUserOutputWsOnNewThread)
                {
                    mThreadManager.add(mThreadId, mUserOutputWs, queueIn++, queueOut++);
                    threadIdPP();
                }
                else
                    mThreadManager.add(mThreadId-1, mUserOutputWs, queueIn++, queueOut++);
            }
            // OpenPose GUI
            if (spWGui != nullptr)
            {
                mThreadManager.add(mThreadId, spWGui, queueIn++, queueOut++);                               // Thread Y+1, queues Q+1 -> Q+2
                threadIdPP();
            }
            log("", Priority::Low, __LINE__, __FUNCTION__, __FILE__);
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    template<typename TDatums, typename TWorker, typename TQueue>
    unsigned int Wrapper<TDatums, TWorker, TQueue>::threadIdPP()
    {
        try
        {
            if (mMultiThreadEnabled)
                mThreadId++;
            return mThreadId;
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    template<typename TDatums, typename TWorker, typename TQueue>
    std::vector<TWorker> Wrapper<TDatums, TWorker, TQueue>::mergeWorkers(const std::vector<TWorker>& workersA, const std::vector<TWorker>& workersB)
    {
        try
        {
            auto workersToReturn(workersA);
            for (auto& worker : workersB)
                workersToReturn.emplace_back(worker);
            return workersToReturn;
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
            return std::vector<TWorker>{};
        }
    }

    extern template class Wrapper<DATUM_BASE_NO_PTR>;
}

#endif // OPENPOSE__WRAPPER__WRAPPER_HPP
