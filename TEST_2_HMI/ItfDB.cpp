// ItfDB.cpp: Implementation of CItfDB Class
//

#include "StdAfx.h"
#include "ItfDB.h"

#define	MAX_COL_LENGTH		50
#define	MAX_FIELD			75	// Maximum number of fields in the read table

#define	DBName(x) (pDBName + (x) * MAX_COL_LENGTH )

//////////////////////////////////////////////////////////////////////
// Constructor/Destructor
//////////////////////////////////////////////////////////////////////
CItfDB::CItfDB(void)
{
	// Table name
	m_strODBC_Connection = "StrAuftraege";
	// User name
    m_strDBUser = "admin";
    // Password                       
	m_strDBPassword = "sqlstring";
}

CItfDB::~CItfDB(void)
{
}

long	CItfDB::UpdateDB(typRecipe *pRecipe)
{
	char		*pDBName = NULL;
	char		*pstrSQL = NULL;
	char		*strCalcYield = NULL;
	int			i;
    BSTR        pErrMsg;

#ifdef DEBUG_FILE
	fDebugFile.open("DB.txt", std::ofstream::out | std::ofstream::app);
#endif

	/* Update Request Strings Allocation */
	pDBName	= (char *) calloc (MAX_FIELD * MAX_COL_LENGTH, sizeof (char)); 
	pstrSQL	= (char *) calloc (MAX_FIELD * MAX_COL_LENGTH, sizeof (char));

	/* Recipe data conversion from long to char * */
	std::ostringstream ossWidth, ossThickness, ossEModule, ossYield;
	ossWidth     << (pRecipe->lngWidth/10.0); // // stored in DB as mm
	ossThickness << (pRecipe->lngThickness/10.0); // stored in DB as mm
	ossEModule   << (pRecipe->lngEModul/1000.0); // stored in DB as kN/mm2

	// Thickness in mm for test
	double thickness = pRecipe->lngThickness/10.0;

	/* Yield Strength Calculation according to table */
	if (pRecipe->strNuance[0] == 'A') // AN42
	{
		if (thickness == 0.41)
			pRecipe->lngYieldStrength = 1100;
		else if (thickness == 0.44)
			pRecipe->lngYieldStrength = 1450;
		else if (thickness == 0.49)
			pRecipe->lngYieldStrength = 320;
		else if (thickness == 0.51)
			pRecipe->lngYieldStrength = 1100;
		else if (thickness == 0.61)
			pRecipe->lngYieldStrength = 840;
		else if (thickness >= 0.71)
			pRecipe->lngYieldStrength = 1100;
		else
			pRecipe->lngYieldStrength = 1200;
	}
	else if (pRecipe->strNuance[0] == 'D') //DS21
	{
			pRecipe->lngYieldStrength = 1080;
#ifdef DEBUG_FILE
		fDebugFile << "DS21!!!!!!!!!!!!!!!!!!" << endl;
#endif
	}
	else if (pRecipe->strNuance[0] == 'M' && pRecipe->strNuance[1] == 'D' && pRecipe->strNuance[2] == '3') //MD30
			pRecipe->lngYieldStrength = 1000;
	else if (pRecipe->strNuance[0] == 'M' && pRecipe->strNuance[1] == 'S' && pRecipe->strNuance[2] == '3') //MS30
			pRecipe->lngYieldStrength = 1400;
	else if (pRecipe->strNuance[0] == 'M' && pRecipe->strNuance[1] == 'S' && pRecipe->strNuance[2] == '2') //MS20
	{
		if(thickness == 0.8)
			pRecipe->lngYieldStrength = 1400;
		else if (thickness >= 1.2)
			pRecipe->lngYieldStrength = 1440;
		else
			pRecipe->lngYieldStrength = 1200;
	}
	else
		pRecipe->lngYieldStrength = 1200;

	ossYield << pRecipe->lngYieldStrength;

	/* Update tblAuftrag Table */

	// Update Request Strings Allocation
	pDBName	= (char *) calloc (MAX_FIELD * MAX_COL_LENGTH, sizeof (char)); 
	pstrSQL	= (char *) calloc (MAX_FIELD * MAX_COL_LENGTH, sizeof (char));

	// Connection to Database
	pErrMsg = cSQL.SQLConnectDB((UCHAR*) m_strODBC_Connection,(UCHAR*)m_strDBUser,(UCHAR*)m_strDBPassword);
	if(pErrMsg)
	{
#ifdef DEBUG_FILE
		fDebugFile << ">>> Error when connect to DB" << endl;
#endif
		return E_FAIL;
	}

#ifdef DEBUG_FILE
	fDebugFile << ">>> Connection to DB OK" << endl;
#endif

	// Update Request Preparation
	strcpy_s (DBName (0), MAX_COL_LENGTH, "UPDATE tblAuftrag SET ");
	strcpy_s (DBName (1), MAX_COL_LENGTH, "strName = ");
	strcpy_s (DBName (2), MAX_COL_LENGTH, pRecipe->strOrderName);
	strcpy_s (DBName (3), MAX_COL_LENGTH, ", strKunde = '");
	strcpy_s (DBName (4), MAX_COL_LENGTH, pRecipe->strCustomerName);
	strcpy_s (DBName (5), MAX_COL_LENGTH, "', strCoilnummer = '");
	strcpy_s (DBName (6), MAX_COL_LENGTH, pRecipe->strCoilNumber);
	strcpy_s (DBName (7), MAX_COL_LENGTH, "', dblDicke = ");
	strcpy_s (DBName (8), MAX_COL_LENGTH, ossThickness.str().c_str());
	strcpy_s (DBName (9), MAX_COL_LENGTH, ", dblMaterialbreite = ");
	strcpy_s (DBName (10), MAX_COL_LENGTH, ossWidth.str().c_str());
	strcpy_s (DBName (11), MAX_COL_LENGTH, " WHERE lngPosition = -1024");
	for(i = 0; i <= 11; i++)
		strcat_s(pstrSQL, MAX_COL_LENGTH*12, DBName(i));

	// Send Update Request
	if(cSQL.SQLSendUpdate((UCHAR*)pstrSQL))
	{
		return E_FAIL;
	}
#ifdef DEBUG_FILE
	fDebugFile << "Send Order Update: " << pstrSQL << endl;
#endif

	// Release Database
	if(cSQL.SQLFree())
		return E_FAIL;

	// Unalloc Update Requests Strings
	free(pDBName);
	free(pstrSQL);

	/* Update tblMaterial Table */

	// Update Request Strings Allocation
	pDBName	= (char *) calloc (MAX_FIELD * MAX_COL_LENGTH, sizeof (char)); 
	pstrSQL	= (char *) calloc (MAX_FIELD * MAX_COL_LENGTH, sizeof (char));

	// Connection to Database
	pErrMsg = cSQL.SQLConnectDB((UCHAR*) m_strODBC_Connection,(UCHAR*)m_strDBUser,(UCHAR*)m_strDBPassword);
	if(pErrMsg)
	{		
		return E_FAIL;
	}

	// Update Request Preparation
	strcpy_s (DBName (0), MAX_COL_LENGTH, "UPDATE tblMaterial SET ");
	strcpy_s (DBName (1), MAX_COL_LENGTH, "dblEModul = ");
	strcpy_s (DBName (2), MAX_COL_LENGTH, ossEModule.str().c_str());
	strcpy_s (DBName (3), MAX_COL_LENGTH, ", dblStreckgrenze = ");
	strcpy_s (DBName (4), MAX_COL_LENGTH, ossYield.str().c_str());
	for(i = 0; i <= 4; i++)
		strcat_s(pstrSQL, MAX_COL_LENGTH*5, DBName(i));

	// Send Update Request
	if(cSQL.SQLSendUpdate((UCHAR*)pstrSQL))
	{
		return E_FAIL;
	}
#ifdef DEBUG_FILE
	fDebugFile << "Send Material Update: " << pstrSQL << endl;
#endif
	// Release Database
	if(cSQL.SQLFree())
		return E_FAIL;

	// Unalloc Update Requests Strings
	free(pDBName);
	free(pstrSQL);

	/* Update tblKassetten Table */

	/* 1- Unselect actual cassette  in use */
	// Update Request Strings Allocation
	pDBName	= (char *) calloc (MAX_FIELD * MAX_COL_LENGTH, sizeof (char)); 
	pstrSQL	= (char *) calloc (MAX_FIELD * MAX_COL_LENGTH, sizeof (char));

	// Connection to Database
	pErrMsg = cSQL.SQLConnectDB((UCHAR*) m_strODBC_Connection,(UCHAR*)m_strDBUser,(UCHAR*)m_strDBPassword);
	if(pErrMsg)
	{		
		return E_FAIL;
	}

	// Update Request Preparation
	strcpy_s (DBName (0), MAX_COL_LENGTH, "UPDATE tblKassetten SET ");
	strcpy_s (DBName (1), MAX_COL_LENGTH, "binEingebaut = FALSE");
	strcpy_s (DBName (2), MAX_COL_LENGTH, " WHERE binEingebaut = TRUE");
	for(i = 0; i <= 2; i++)
		strcat_s(pstrSQL, MAX_COL_LENGTH*3, DBName(i));

	// Send Update Request
	if(cSQL.SQLSendUpdate((UCHAR*)pstrSQL))
	{
		return E_FAIL;
	}
#ifdef DEBUG_FILE
	fDebugFile << "Send Old Cassette in use Update: " << pstrSQL << endl;
#endif
	// Release Database
	if(cSQL.SQLFree())
		return E_FAIL;

	// Unalloc Update Requests Strings
	free(pDBName);
	free(pstrSQL);

	/* 2- Select new cassette  in use */
	// Update Request Strings Allocation
	pDBName	= (char *) calloc (MAX_FIELD * MAX_COL_LENGTH, sizeof (char)); 
	pstrSQL	= (char *) calloc (MAX_FIELD * MAX_COL_LENGTH, sizeof (char));

	// Connection to Database
	pErrMsg = cSQL.SQLConnectDB((UCHAR*) m_strODBC_Connection,(UCHAR*)m_strDBUser,(UCHAR*)m_strDBPassword);
	if(pErrMsg)
	{		
		return E_FAIL;
	}

	// Update Request Preparation
	strcpy_s (DBName (0), MAX_COL_LENGTH, "UPDATE tblKassetten SET ");
	strcpy_s (DBName (1), MAX_COL_LENGTH, "binEingebaut = TRUE");
	strcpy_s (DBName (2), MAX_COL_LENGTH, " WHERE strKassettenbezeichnung = '");
	strcpy_s (DBName (3), MAX_COL_LENGTH, pRecipe->strCassetteName);
	strcpy_s (DBName (4), MAX_COL_LENGTH, "'");
	for(i = 0; i <= 4; i++)
		strcat_s(pstrSQL, MAX_COL_LENGTH*5, DBName(i));

	// Send Update Request
	if(cSQL.SQLSendUpdate((UCHAR*)pstrSQL))
	{
		return E_FAIL;
	}
#ifdef DEBUG_FILE
	fDebugFile << "Send New Cassette in use Update: " << pstrSQL << endl;
#endif
	// Release Database
	if(cSQL.SQLFree())
	{
#ifdef DEBUG_FILE
	fDebugFile << "Fail release " << endl;
#endif
		return E_FAIL;
	}

	// Unalloc Update Requests Strings
	free(pDBName);
	free(pstrSQL);

#ifdef DEBUG_FILE
	fDebugFile << ">>> Update Finished" << endl;
	fDebugFile.close();
#endif

	return S_OK;
}