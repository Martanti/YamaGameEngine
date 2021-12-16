#ifdef GAME
#include "HighscoreWrapper.h"
using namespace curlpp::options;
std::vector<HighScoreEntity> HighscoreWrapper::GetTopScore()
{
    std::vector<HighScoreEntity> highscores{};
#ifdef NO_DEF
    highscores.reserve(5);
    try
    {
        curlpp::Cleanup myCleanup;

        curlpp::Easy request;
        request.setOpt<Url>(mWebApiUrl);
        std::ostringstream os;
        curlpp::options::WriteStream ws(&os);
        request.setOpt(ws);
        request.perform();

        auto str = os.str();

        Log(LGR_INFO, std::format("Output from received top score call: {}", str));

        json aa = json::parse(str);

        for (auto& item : aa)
        {
            auto name = item["Name"].get<std::string>();
            auto score = item["Score"].get<int>();
            auto date = item["Date"].get<std::string>();

            highscores.emplace_back(HighScoreEntity{ .mScore = score, .mName = name, .mDate = date });
        }
    }

    catch (curlpp::RuntimeError const& e)
    {
        Log(LGR_ERROR, std::format("Runtime curlpp error while getting top scores: {} ", e.what()));
    }

    catch (curlpp::LogicError const& e)
    {
        Log(LGR_ERROR, std::format("Logic curlpp error while getting top scores: {} ", e.what()));
    }

    catch (std::exception const& e)
    {
        Log(LGR_ERROR, std::format("Error while getting top scores: {} ", e.what()));
    }
#else
    Log(LGR_WARNING, "The online highscore capabilieties were turned off for the public repository");
#endif // NO_DEF
    return highscores;
}

void HighscoreWrapper::UploadHighscore(HighScoreEntity const& score)
{
    try
    {
#ifdef NO_DEF
        curlpp::Cleanup cleaner;
        curlpp::Easy request;

        request.setOpt(new curlpp::options::Url(mWebApiUrl));
        std::ostringstream os;
        curlpp::options::WriteStream ws(&os);
        request.setOpt(ws);
        {
            // Forms takes ownership of pointers
            curlpp::Forms formParts;
            auto scoreTemp = std::to_string(score.mScore);
            formParts.push_back(new curlpp::FormParts::Content("Name", score.mName.c_str()));
            formParts.push_back(new curlpp::FormParts::Content("Score", scoreTemp.c_str()));

            request.setOpt(new curlpp::options::HttpPost(formParts));
        }

        request.perform();

        Log(LGR_INFO, std::format("Output from received top score call: {}", os.str()));
#else
        Log(LGR_WARNING, "The online highscore capabilities were turned off for the public repository");
#endif // NO_DEF

    }
    catch (curlpp::RuntimeError const& e)
    {
        Log(LGR_ERROR, std::format("Runtime curlpp error while getting top scores: {} ", e.what()));
    }

    catch (curlpp::LogicError const& e)
    {
        Log(LGR_ERROR, std::format("Logic curlpp error while getting top scores: {} ", e.what()));
    }

    catch (std::exception const& e)
    {
        Log(LGR_ERROR, std::format("Error while getting top scores: {} ", e.what()));
    }

}
#endif // DEBUG
