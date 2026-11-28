#include "gamepadui_interface.h"
#include "gamepadui_basepanel.h"
#include "gamepadui_mainmenu.h"

#include "vgui/ISurface.h"
#include "vgui/ILocalize.h"
#include "vgui/IVGui.h"

#include "KeyValues.h"
#include "convar.h"
#include "filesystem.h"

#include "inputsystem/iinputsystem.h"
#include "tier2/tier2.h"

#include "tier0/memdbgon.h"

#define GAMEPADUI_MAINMENU_SCHEME GAMEPADUI_RESOURCE_FOLDER "schememainmenu.res"
#define GAMEPADUI_MAINMENU_FILE GAMEPADUI_RESOURCE_FOLDER "mainmenu.res"

GamepadUIMainMenu::GamepadUIMainMenu( vgui::Panel* pParent )
    : BaseClass( pParent, "MainMenu" )
{
    vgui::HScheme hScheme = vgui::scheme()->LoadSchemeFromFile( GAMEPADUI_MAINMENU_SCHEME, "SchemeMainMenu" );
    SetScheme( hScheme );

    KeyValues* pModData = new KeyValues( "ModData" );
    if ( pModData )
    {
        if ( pModData->LoadFromFile( g_pFullFileSystem, "gameinfo.txt" ) )
        {
            m_LogoText[ 0 ] = pModData->GetString( "gamepadui_title", pModData->GetString( "title" ) );
            m_LogoText[ 1 ] = pModData->GetString( "gamepadui_title2", pModData->GetString( "title2" ) );
        }
        pModData->deleteThis();
    }

    LoadMenuButtons();

    SetFooterButtons( FooterButtons::Select, FooterButtons::Select );
}

void GamepadUIMainMenu::UpdateGradients()
{
    const float flTime = GamepadUI::GetInstance().GetTime();
    GamepadUI::GetInstance().GetGradientHelper()->ResetTargets( flTime );
    GamepadUI::GetInstance().GetGradientHelper()->SetTargetGradient( GradientSide::Left, { 1.0f, 0.666f }, flTime );
}

// Small ">_" icon in the bottom right corner that opens / closes the console.
// Drawn with plain vgui primitives so it needs no material or other asset.
// Hidden while a physical keyboard is attached: the ~ key can open the console.
class GamepadUIConsoleIcon : public vgui::Panel
{
    DECLARE_CLASS_SIMPLE( GamepadUIConsoleIcon, vgui::Panel );
public:
    GamepadUIConsoleIcon( vgui::Panel *pParent ) : BaseClass( pParent, "ConsoleIcon" )
    {
        SetMouseInputEnabled( true );
        SetKeyBoardInputEnabled( false );
        SetPaintBackgroundEnabled( false );
        SetZPos( 200 );
    }

    void OnThink() OVERRIDE
    {
        BaseClass::OnThink();

        int nParentW, nParentH;
        GetParent()->GetSize( nParentW, nParentH );
        int nSize = MAX( 48, nParentH / 12 );
        int nMargin = nSize / 3;
        // Keep clear of the footer button row that also lives bottom-right.
        SetBounds( nParentW - nSize - nMargin, nParentH - nSize - nMargin - nSize * 2 / 3, nSize, nSize );
    }

    void Paint() OVERRIDE
    {
        int w, h;
        GetSize( w, h );
        const bool bHover = IsCursorOver();

        vgui::surface()->DrawSetColor( 0, 0, 0, bHover ? 200 : 130 );
        vgui::surface()->DrawFilledRect( 0, 0, w, h );

        const int nA = bHover ? 255 : 200;
        vgui::surface()->DrawSetColor( 255, 255, 255, nA );
        vgui::surface()->DrawOutlinedRect( 0, 0, w, h );

        // chevron ">"
        const int t = MAX( 2, w / 16 );
        const int x0 = w * 22 / 100, x1 = w * 46 / 100;
        const int yc = h / 2;
        const int dy = h * 16 / 100;
        for ( int i = 0; i < t; i++ )
        {
            vgui::surface()->DrawLine( x0 + i, yc - dy, x1 + i, yc );
            vgui::surface()->DrawLine( x0 + i, yc + dy, x1 + i, yc );
        }
        // cursor "_"
        vgui::surface()->DrawFilledRect( w * 54 / 100, yc + dy - t, w * 78 / 100, yc + dy );
    }

    void OnMouseReleased( vgui::MouseCode code ) OVERRIDE
    {
        BaseClass::OnMouseReleased( code );
        if ( code != MOUSE_LEFT )
            return;

        IGameConsole *pConsole = GamepadUI::GetInstance().GetGameConsole();
        if ( !pConsole )
            return;

        if ( pConsole->IsConsoleVisible() )
            pConsole->Hide();
        else
            pConsole->Activate();
    }
};

void GamepadUIMainMenu::LoadMenuButtons()
{
    KeyValues* pDataFile = new KeyValues( "MainMenuScript" );
    if ( pDataFile )
    {
        if ( pDataFile->LoadFromFile( g_pFullFileSystem, GAMEPADUI_MAINMENU_FILE ) )
        {
            for ( KeyValues* pData = pDataFile->GetFirstSubKey(); pData != NULL; pData = pData->GetNextKey() )
            {
                GamepadUIButton* pButton = new GamepadUIButton(
                    this, this,
                    GAMEPADUI_MAINMENU_SCHEME,
                    pData->GetString( "command" ),
                    pData->GetString( "text", "Sample Text" ),
                    pData->GetString( "description", "" ) );
                pButton->SetPriority( V_atoi( pData->GetString( "priority", "0" ) ) );
                pButton->SetVisible( true );

                const char* pFamily = pData->GetString( "family", "all" );
                if ( !V_strcmp( pFamily, "ingame" ) || !V_strcmp( pFamily, "all" ) )
                    m_Buttons[ GamepadUIMenuStates::InGame ].AddToTail( pButton );
                if ( !V_strcmp( pFamily, "mainmenu" ) || !V_strcmp( pFamily, "all" ) )
                    m_Buttons[ GamepadUIMenuStates::MainMenu ].AddToTail( pButton );
            }
        }

        pDataFile->deleteThis();
    }

    {
        ConVarRef merged( "merged_games", true );
        m_bMerged = merged.IsValid() && merged.GetBool();
    }

    if ( m_bMerged )
    {
        // Priority is ascending from the bottom of the screen (QUIT is lowest).
        // The page buttons live in their own lists, so switching pages is just a visibility swap.
        struct MergedEntry_t { const char *pText; const char *pCommand; int nPriority; };
        static const MergedEntry_t s_Chooser[] =
        {
            { "QUIT",             "cmd gamepadui_openquitgamedialog",        1 },
            { "OPTIONS",          "cmd gamepadui_openoptionsdialog",         2 },
            { "EXTRAS",           "cmd gamepadui_openachievementsdialog",    3 },
            { "HL:2 EPISODE TWO", "merged ep2",                              4 },
            { "HL:2 EPISODE ONE", "merged episodic",                         5 },
            { "HALF-LIFE 2",      "merged hl2",                              6 },
        };
        for ( const MergedEntry_t &e : s_Chooser )
        {
            GamepadUIButton *pButton = new GamepadUIButton( this, this, GAMEPADUI_MAINMENU_SCHEME, e.pCommand, e.pText, "" );
            pButton->SetPriority( e.nPriority );
            pButton->SetVisible( false );
            m_Buttons[ GamepadUIMenuStates::Chooser ].AddToTail( pButton );
        }

        // Back to the game list, shown at the top of a game's own menu (not in a level)
        GamepadUIButton *pBack = new GamepadUIButton( this, this, GAMEPADUI_MAINMENU_SCHEME, "mergedback", "BACK", "" );
        pBack->SetPriority( 100 );
        pBack->SetVisible( false );
        m_Buttons[ GamepadUIMenuStates::MainMenu ].AddToTail( pBack );

        m_bMergedChooser = true;
    }

    m_pConsoleIcon = new GamepadUIConsoleIcon( this );

    UpdateButtonVisibility();
}

void GamepadUIMainMenu::ApplySchemeSettings( vgui::IScheme* pScheme )
{
    BaseClass::ApplySchemeSettings( pScheme );

    int nParentW, nParentH;
	GetParent()->GetSize( nParentW, nParentH );
    SetBounds( 0, 0, nParentW, nParentH );

    const char *pImage = pScheme->GetResourceString( "Logo.Image" );
    if ( pImage && *pImage )
        m_LogoImage.SetImage( pImage );
    m_hLogoFont = pScheme->GetFont( "Logo.Font", true );
}

void GamepadUIMainMenu::LayoutMainMenu()
{
    int nY = GetCurrentButtonOffset();
    CUtlVector<GamepadUIButton*>& currentButtons = GetCurrentButtons();
    for ( GamepadUIButton *pButton : currentButtons )
    {
        nY += pButton->GetTall();
        pButton->SetPos( m_flButtonsOffsetX, GetTall() - nY );
        nY += m_flButtonSpacing;
    }
}

void GamepadUIMainMenu::PaintLogo()
{
    vgui::surface()->DrawSetTextColor( m_colLogoColor );
    vgui::surface()->DrawSetTextFont( m_hLogoFont );

    int nMaxLogosW = 0, nTotalLogosH = 0;
    int nLogoW[ 2 ], nLogoH[ 2 ];
    for ( int i = 0; i < 2; i++ )
    {
        nLogoW[ i ] = 0;
        nLogoH[ i ] = 0;
        if ( !m_LogoText[ i ].IsEmpty() )
            vgui::surface()->GetTextSize( m_hLogoFont, m_LogoText[ i ].String(), nLogoW[ i ], nLogoH[ i ] );
        nMaxLogosW = Max( nLogoW[ i ], nMaxLogosW );
        nTotalLogosH += nLogoH[ i ];
    }

    int nLogoY = GetTall() - ( GetCurrentLogoOffset() + nTotalLogosH );

    if ( m_LogoImage.IsValid() )
    {
        int nY1 = nLogoY;
        int nY2 = nY1 + nLogoH[ 0 ];
        int nX1 = m_flLogoOffsetX;
        int nX2 = nX1 + ( nLogoH[ 0 ] * 3 );
        vgui::surface()->DrawSetColor( Color( 255, 255, 255, 255 ) );
        vgui::surface()->DrawSetTexture( m_LogoImage );
        vgui::surface()->DrawTexturedRect( nX1, nY1, nX2, nY2 );
        vgui::surface()->DrawSetTexture( 0 );
    }
    else
    {
        for ( int i = 1; i >= 0; i-- )
        {
            vgui::surface()->DrawSetTextPos( m_flLogoOffsetX, nLogoY );
            vgui::surface()->DrawPrintText( m_LogoText[ i ].String(), m_LogoText[ i ].Length() );

            nLogoY -= nLogoH[ i ];
        }
    }
}

void GamepadUIMainMenu::OnThink()
{
    BaseClass::OnThink();

    // The icon is hidden while a physical keyboard is attached. Update it from
    // the menu, because a hidden child does not get Think() called.
    if ( m_pConsoleIcon )
        m_pConsoleIcon->SetVisible( !g_pInputSystem || !g_pInputSystem->IsPhysicalKeyboardConnected() );

    LayoutMainMenu();
}

void GamepadUIMainMenu::Paint()
{
    BaseClass::Paint();

    PaintLogo();
}

void GamepadUIMainMenu::OnCommand( char const* pCommand )
{
    if ( StringHasPrefixCaseSensitive( pCommand, "cmd " ) )
    {
        const char* pszClientCmd = &pCommand[ 4 ];
        if ( *pszClientCmd )
            GamepadUI::GetInstance().GetEngineClient()->ClientCmd_Unrestricted( pszClientCmd );
    }
    else if ( m_bMerged && StringHasPrefixCaseSensitive( pCommand, "merged " ) )
    {
        // Switch the mounted content (no restart, no map load) and show that game's own menu
        char szCmd[96];
        V_snprintf( szCmd, sizeof( szCmd ), "merged_switch %s", &pCommand[ 7 ] );
        GamepadUI::GetInstance().GetEngineClient()->ClientCmd_Unrestricted( szCmd );
        m_bMergedChooser = false;
        OnMenuStateChanged();
    }
    else if ( m_bMerged && !V_strcmp( pCommand, "mergedback" ) )
    {
        m_bMergedChooser = true;
        OnMenuStateChanged();
    }
    else
    {
        BaseClass::OnCommand( pCommand );
    }
}

void GamepadUIMainMenu::OnSetFocus()
{
    BaseClass::OnSetFocus();
    OnMenuStateChanged();
}

void GamepadUIMainMenu::OnMenuStateChanged()
{
    UpdateGradients();
    UpdateButtonVisibility();
}

void GamepadUIMainMenu::UpdateButtonVisibility()
{
    for ( CUtlVector<GamepadUIButton*>& buttons : m_Buttons )
    {
        for ( GamepadUIButton* pButton : buttons )
        {
            pButton->NavigateFrom();
            pButton->SetVisible( false );
        }
    }

    CUtlVector<GamepadUIButton*>& currentButtons = GetCurrentButtons();
    currentButtons.Sort( []( GamepadUIButton* const* a, GamepadUIButton* const* b ) -> int
    {
        return ( ( *a )->GetPriority() > ( *b )->GetPriority() );
    });

    for ( int i = 1; i < currentButtons.Count(); i++ )
    {
        currentButtons[i]->SetNavDown( currentButtons[i - 1] );
        currentButtons[i - 1]->SetNavUp( currentButtons[i] );
    }

    for ( GamepadUIButton* pButton : currentButtons )
        pButton->SetVisible( true );

    if ( !currentButtons.IsEmpty() )
        currentButtons[ currentButtons.Count() - 1 ]->NavigateTo();
}

void GamepadUIMainMenu::OnKeyCodeReleased( vgui::KeyCode code )
{
    ButtonCode_t buttonCode = GetBaseButtonCode( code );
    switch (buttonCode)
    {
#ifdef HL2_RETAIL // Steam input and Steam Controller are not supported in SDK2013 (Madi)
    case STEAMCONTROLLER_B:
#endif

    case KEY_XBUTTON_B:
        if ( GamepadUI::GetInstance().IsInLevel() )
        {
            GamepadUI::GetInstance().GetEngineClient()->ClientCmd_Unrestricted( "gamemenucommand resumegame" );
            // I tried it and didn't like it.
            // Oh well.
            //vgui::surface()->PlaySound( "UI/buttonclickrelease.wav" );
        }
        break;
    default:
        BaseClass::OnKeyCodeReleased( code );
        break;
    }
}

GamepadUIMenuState GamepadUIMainMenu::GetCurrentMenuState() const
{
    if ( GamepadUI::GetInstance().IsInLevel() )
        return GamepadUIMenuStates::InGame;
    if ( m_bMerged && m_bMergedChooser )
        return GamepadUIMenuStates::Chooser;
    return GamepadUIMenuStates::MainMenu;
}

CUtlVector<GamepadUIButton*>& GamepadUIMainMenu::GetCurrentButtons()
{
    return m_Buttons[ GetCurrentMenuState() ];
}

float GamepadUIMainMenu::GetCurrentButtonOffset()
{
    return GetCurrentMenuState() == GamepadUIMenuStates::InGame ? m_flButtonsOffsetYInGame : m_flButtonsOffsetYMenu;
}

float GamepadUIMainMenu::GetCurrentLogoOffset()
{
    return GetCurrentMenuState() == GamepadUIMenuStates::InGame ? m_flLogoOffsetYInGame : m_flLogoOffsetYMenu;
}
