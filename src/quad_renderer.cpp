#include "quad_renderer.hpp"
#include "tz/topaz.hpp"
#include "tz/core/vector.hpp"
#include "tz/gpu/resource.hpp"
#include "tz/gpu/pass.hpp"
#include "tz/gpu/graph.hpp"
#include "tz/gpu/shader.hpp"

#include ImportedShaderHeader(quad, vertex)
#include ImportedShaderHeader(quad, fragment)

namespace quad
{
	tz::gpu::resource_handle quad_buffer = tz::nullhand;

	tz::gpu::pass_handle render_pass = tz::nullhand;
	tz::gpu::graph_handle graph = tz::nullhand;

	constexpr std::size_t initial_quad_limit = 1024u;
	std::size_t quad_count = 0;

	struct quad_data
	{
		tz::v3f pos;
		float pad0;
		tz::v3f scale;
		float pad1;
	};

	void setup()
	{
		std::array<quad_data, initial_quad_limit> initial_quad_data;
		std::fill(initial_quad_data.begin(), initial_quad_data.end(), quad_data{});
		quad_buffer = tz_must(tz::gpu::create_buffer
		({
			.data = std::as_bytes(std::span<const quad_data>(initial_quad_data)) ,
			.flags = tz::gpu::buffer_flag::dynamic_access
		}));

		tz::gpu::resource_handle colour_targets[] =
		{
			tz::gpu::window_resource
		};

		render_pass = tz_must(tz::gpu::create_pass
		({
			.graphics =
			{
				.clear_colour = {0.5f, 0.5f, 1.0f, 1.0f},
				.colour_targets = colour_targets,
				.flags = tz::gpu::graphics_flag::no_depth_test,
				.triangle_count = quad_count * 2
			},
			.shader = tz_must(tz::gpu::create_graphics_shader(ImportedShaderSource(quad, vertex), ImportedShaderSource(quad, fragment))),
		}));

		graph = tz_must(tz::gpu::graph_builder{}
		.set_flags(tz::gpu::graph_flag::present_after)
		.add_pass(render_pass)
		.build()
		);
	}

	tz::gpu::graph_handle renderer()
	{
		return graph;
	}
}