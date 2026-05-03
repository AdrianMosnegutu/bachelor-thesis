target_sources(dsl_lowering PRIVATE
        lower_definitions.cpp
        lower_declarative_statements.cpp
        lower_control_flow_statements.cpp
        lower_play_statement.cpp
        lower_statement.cpp
)
